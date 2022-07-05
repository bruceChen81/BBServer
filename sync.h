#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

#include "common.h"
#include "queue.h"

typedef enum syncState
{
    SYNC_DISCONNECT = 1,

    SYNC_IDLE, //2 connected

    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //3 write request from a client, broadcasted precommit
    SYNC_M_PRECOMMITED,           //4 positive acked by everybody
    SYNC_M_PRECOMMIT_UNSUCCESS,   //5
    SYNC_M_COMMITED,              //6 broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //7 positive acked by everybody, performed operation
    SYNC_M_OPERATION_UNSUCCESS,   //8

    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //9 received precommit
    SYNC_S_PRECOMMIT_ACK,      //10 sent positive ack
    SYNC_S_COMMITED,           //11 received commit, performed operation, send successful
    SYNC_S_UNDO,               //12 operation unsuccessful, undo


    //user client
    SYNC_U_WAITING_COMMIT, //13
    SYNC_U_WAITING_SAVE,   //14   
    SYNC_U_SAVING,         //15
    SYNC_U_SAVED,          //16

    SYNC_MAX

}syncState;


int init_sync_server_list();

int init_sync_server_connection();

int sync_connect_to_server(std::string& ip, unsigned int port);

int sync_send_precommit();

int sync_send_abort();

int sync_send_commit(clientCmdType type, std::string& msgbody);

int sync_send_success(bool isSuccessful, std::string& msgNumber);

int sync_send_event(clientEvType type, std::string& msgNumber, int fd, std::string& msg);

int sync_send_event_timeout(clientEvType type);


bool sync_check_server_state(syncState state);

bool sync_check_service_enable();


//state timer
//master
int create_timer_master();

int start_timer_master(int sec);

int stop_timer_master();

int delete_timer_master();

//slave
int create_timer_slave();

int start_timer_slave(int sec);

int stop_timer_slave();

int delete_timer_slave();

void destroy_timer();

#endif // SYNC_H_INCLUDED
