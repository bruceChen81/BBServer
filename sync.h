#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

#include "common.h"
#include "queue.h"

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
    SYNC_U_SAVING,
    SYNC_U_SAVED,

    SYNC_MAX

}syncState;


int init_sync_server_list();

int init_sync_server_connection();

int sync_send_precommit();

int sync_send_abort();

int sync_send_commit(clientCmdType type, std::string& msgbody);

int sync_send_success(bool isSuccessful, std::string& msgNumber);


int sync_send_event(clientEv type, std::string& msgNumber, int fd, std::string& msg);

int sync_send_event_timeout(clientEv type);


bool sync_check_server_state(syncState state);

bool sync_check_service_enable();



int sync_connect_to_server(std::string& ip, unsigned int port);




int create_timer_master();

int start_timer_master(int sec);

int stop_timer_master();

int delete_timer_master();


int create_timer_slave();

int start_timer_slave(int sec);

int stop_timer_slave();

int delete_timer_slave();

void destroy_timer();

#endif // SYNC_H_INCLUDED
