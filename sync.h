#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

#include "common.h"

typedef enum syncState
{
    SYNC_DISCONNECT = 1,

    SYNC_IDLE, //connected

    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //3 write request from a client, broadcasted precommit
    SYNC_M_PRECOMMITED,           //4 positive acked by everybody
    SYNC_M_PRECOMMIT_UNSUCCESS,
    SYNC_M_COMMITED,              //5 broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //6 positive acked by everybody, performed operation
    SYNC_M_OPERATION_UNSUCCESS,

    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //7 received precommit
    SYNC_S_PRECOMMIT_ACK,      //8 sent positive ack
    SYNC_S_COMMITED,           //9 received commit, performed operation, send successful
    SYNC_S_UNDO,               //10 operation unsuccessful, undo


    //user client
    SYNC_U_WAITING_COMMIT,
    SYNC_U_WAITING_SAVE,

    SYNC_MAX

}syncState;


int init_sync_server_list();

int init_sync_server_connection();

int sync_send_precommit();

int sync_send_abort();

int sync_send_commit(clientCmdType type, std::string& msgbody);

int sync_send_success(bool isSuccessful, std::string& msgNumber);

syncState sync_get_master_state();

bool sync_check_server_state(syncState state);

bool sync_check_service_enable();



int sync_connect_to_server(std::string& ip, unsigned int port);


timer_t start_timer(int t_second);

#endif // SYNC_H_INCLUDED
