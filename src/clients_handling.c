#include "../headers/clients_handling.h"

int make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

void read_data_from_socket(int client_socket, void *receive_buffer, int size){
  int accumuleted_received_data_count = 0;
  int current_read_data_count = 0;

  while(accumuleted_received_data_count < sizeof(struct RandomAccessPreamble)){
    current_read_data_count = read(client_socket, receive_buffer + accumuleted_received_data_count, sizeof(struct RandomAccessPreamble)-accumuleted_received_data_count);
    if(current_read_data_count > 0){
      accumuleted_received_data_count += current_read_data_count;
    }
  }
}

void handle_new_connection(int server_socket){
  struct sockaddr_in client_address;
  int client_address_len = sizeof(client_address);
  int client_socket = accept(server_socket,
                     (struct sockaddr *) &client_address, &client_address_len);
  if (client_socket == -1) {
      perror("accept");
      exit(EXIT_FAILURE);
  }
  make_socket_non_blocking(client_socket);

  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
  ev.data.fd = client_socket;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_socket,
              &ev) == -1) {
      perror("epoll_ctl");
      exit(EXIT_FAILURE);
  }
  printf("New connection with fd: %d\n", client_socket);

  //handle LTE random access
  lte_random_access_procedure(client_socket);
  lte_rrc_connection_establishment(client_socket);
}

void handle_client(int fd){
  char rec_msg_buffer[MAX_MSG_LEN];
  char rec_char_buffer;
  int rec_char_index = 0;

  while(read(fd, &rec_char_buffer, 1) > 0){
    rec_msg_buffer[rec_char_index++] = rec_char_buffer;
    if(rec_char_index >= MAX_MSG_LEN){
      sprintf(rec_msg_buffer, "Message is too long");
      break;
    }
    if(rec_char_buffer == '\0'){
      break;
    }
  }
  if(rec_char_index == 0){
    /*printf("Client %d closed connection\n", fd);
    del_connected_client(fd);
    printf("Current connected clients number: %d", connected_clients_number);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd,NULL);
    close(fd);*/
    return;
  }
  printf("Client %d sent: %s", fd, rec_msg_buffer);
  printf("\n");
  printf("Server responding to client %d: %s", fd, rec_msg_buffer);
  printf("\n");
  write(fd, rec_msg_buffer, strlen(rec_msg_buffer)+1);
}

void server_run(int argc, char** argv)
{

  //server and client addressess
  unsigned short PORT = atoi(argv[1]);
	struct sockaddr_in server_address;
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);


  if(server_socket == -1){
    perror("socket");
    exit(EXIT_FAILURE);
  }

  //setting server to nonblocking
  make_socket_non_blocking(server_socket);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(PORT);

  if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0){
    perror("bind");
    exit(EXIT_FAILURE);
  }

  printf("Server up\n");

  if(listen(server_socket, 5) == -1){
   perror("listen");
   exit(EXIT_FAILURE);
  }

  printf("Listening..\n");

  epollfd = epoll_create1(0);
   if (epollfd == -1) {
       perror("epoll_create1");
       exit(EXIT_FAILURE);
   }
   ev.events = EPOLLIN;
   ev.data.fd = server_socket;
   if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
     perror("epoll_ctl");
     exit(EXIT_FAILURE);
   }

  while(1) {
      nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    //  printf("test\n");
      if (nfds == -1) {
          perror("epoll_wait");
          exit(EXIT_FAILURE);
      }

      for (int n = 0; n < nfds; ++n) {
          if (events[n].data.fd == server_socket) {
            handle_new_connection(server_socket);
          } else {
              if(events[n].events & (EPOLLRDHUP | EPOLLHUP)){
                int fd = events[n].data.fd;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, fd,NULL);
                printf("Closing connection with fd: %d\n", fd);
                del_connected_client(fd);
                printf("Current connected clients number: %d\n", connected_clients_number);
                close (fd);
              }
              if(events[n].events & EPOLLIN){
                handle_client(events[n].data.fd);
              }
          }
      }
  }

  printf("Server down\n");
  close(server_socket);
}
