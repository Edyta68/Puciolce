#ifndef SERVER_DATA_H_
#define SERVER_DATA_H_

#include <stdio.h>
#include <time.h>

#include "preambles.h"
#include "hash_table.h"
#include "download.h"
#include "rrc_connection.h"
#include "service_X2_handover.h"

#define MAX_CLIENTS 256

//errors
#define ERR_ADD_CC_OVERFLOW -1
#define ERR_ADD_CC_ALREADY_CONNECTED -2
#define ERR_GET_CC_NO_MATCH -1
#define ERR_DEL_CC_NO_MATCH -1

typedef struct X2_Measurment_Status_Info{
    clock_t last_request_time;
    int reported_signal; // in range 0 - 100
}X2_Measurment_Status_Info;

typedef struct ping_info {
  bool low_battery_level;
  clock_t last_request_time;
  clock_t last_response_time;
} ping_info;

typedef struct Download_Status{
  bool in_progress;
  Download_Info info;
  int current_packet_index;
  int file_descriptor;
}Download_Status;

typedef struct connected_client{
  int temp_c_rnti;
  Sequence sequence;
  ping_info ping;
  Download_Status download;
  RRC_Connection_Request connection_request;
  RRC_Connection_Setup_Complete setup_complete;
  X2_Measurment_Status_Info measurment_status;
}connected_client;

extern Hash_Table *connected_clients;
extern int connected_clients_number;

connected_client *get_connected_client(int temp_c_rnti);
int add_connected_client(int temp_c_rnti, Sequence sequence, RRC_Connection_Request connection_request, RRC_Connection_Setup_Complete setup_complete);
int del_connected_client(int temp_c_rnti);
void initialize_table();

#endif
