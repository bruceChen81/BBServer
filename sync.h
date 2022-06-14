#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

typedef enum syncState
{
    SYNC_DISCONNECT = 1,

    SYNC_IDLE, //connected

    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //3 write request from a client, broadcasted precommit
    SYNC_M_COMMITED,              //4 positive acked by everybody, broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //5 positive acked by everybody, performed operation

    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //received precommit
    SYNC_S_PRECOMMIT_ACK,      //6 sent positive ack
    SYNC_S_COMMITED,           //7 received commit, performed operation, send successful
    SYNC_S_UNDO                //operation unsuccessful, undo

}syncState;


int init_sync_server_list();

int init_sync_server_connection();

int sync_send_precommit();

int sync_send_commit(std::string& msgbody);


bool sync_check_server_state(syncState state);


void *handle_data_sync_event(void *arg);

int sync_connect_to_server(std::string& ip, unsigned int port);


timer_t start_timer(int t_second);

#endif // SYNC_H_INCLUDED
