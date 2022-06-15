#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

typedef enum syncState
{
    SYNC_DISCONNECT = 1,

    SYNC_IDLE, //connected

    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //3 write request from a client, broadcasted precommit
    SYNC_M_PRECOMMITED,           //4 positive acked by everybody
    SYNC_M_COMMITED,              //5 broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //6 positive acked by everybody, performed operation

    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //7 received precommit
    SYNC_S_PRECOMMIT_ACK,      //8 sent positive ack
    SYNC_S_COMMITED,           //9 received commit, performed operation, send successful
    SYNC_S_UNDO                //10 operation unsuccessful, undo

}syncState;


int init_sync_server_list();

int init_sync_server_connection();

int sync_send_precommit();

int sync_send_commit(std::string& msgbody);

int sync_send_success(bool isSuccessful);


bool sync_check_server_state(syncState state);


void *handle_data_sync_event(void *arg);

int sync_connect_to_server(std::string& ip, unsigned int port);


timer_t start_timer(int t_second);

#endif // SYNC_H_INCLUDED
