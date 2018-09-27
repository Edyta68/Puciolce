#include "clients_services.h"

pthread_t services_thread;

void *run_services(void *unused){

  setbuf(stdout, NULL);
  while(server_running){
    iter_Hash(connected_clients, handle_client_services);
    print_logs_to_console();
    usleep(SERVICES_SLEEP_TIME*1000); //converting to microseconds
  }
  pthread_exit(NULL);
  return NULL;
}

void handle_client_services(int temp_c_rnti, void *client){
  if(ping_client(client)){
    return;
  }
  handle_client_download(client);
  handle_measurment_control(client);
}
