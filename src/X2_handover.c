#include "X2_handover.h"

X2_Server_Info other_server_info = {0};
bool other_server_connected = false;
int other_server_fd = 0;

int x2_request_server_connection(struct sockaddr_in server_address){
  other_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(other_server_fd == -1){
    return ERR_X2_SOCKET_ERR;
  }
  if(connect(other_server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) != -1) {
    message_label request_label = {
      message_type: msg_x2_server_connection_request,
      message_length: sizeof(other_server_info)
    };
    write(other_server_fd, &request_label, sizeof(request_label));
    write(other_server_fd, &server_info, sizeof(server_info));

    message_label response_label = {};
    int response_status = 0;
    if(read_data_from_socket(other_server_fd, &response_label, sizeof(response_label)) < sizeof(response_label)){
      return ERR_X2_READ_TIMOUT;
    }
    if(response_label.message_type != msg_x2_server_connection_response || response_label.message_length != sizeof(response_status)){
      return ERR_X2_DATA_MISMATCH;
    }
    if(read_data_from_socket(other_server_fd, &response_status, sizeof(response_status)) < sizeof(response_status)){
      return ERR_X2_READ_TIMOUT;
    }
    return response_status;
  }
  else{
    return ERR_X2_SOCKET_ERR;
  }
  return 0;
}

int x2_handle_server_connection(int client_socket){
  if(read_data_from_socket(client_socket, &other_server_info, sizeof(other_server_info)) < sizeof(other_server_info)){
    return ERR_X2_READ_TIMOUT;
  }

  int x2_response = ERR_X2_SERVER_CONNECTION_ESTABLISHED;
  message_label response_label = {
    message_type: msg_x2_server_connection_response,
    message_length: sizeof(x2_response)
  };
  write(client_socket, &response_label, sizeof(response_label));

  if(other_server_connected){
    x2_response = ERR_X2_OTHER_SERVER_CONNECTED;
    write(client_socket, &x2_response, sizeof(x2_response));
    return ERR_X2_OTHER_SERVER_CONNECTED;
  }
  write(client_socket, &x2_response, sizeof(x2_response));
  other_server_fd = client_socket;
  return ERR_X2_SERVER_CONNECTION_ESTABLISHED;
}
