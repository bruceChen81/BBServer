#ifndef SYNC_H_INCLUDED
#define SYNC_H_INCLUDED

typedef enum syncServerState
{
    SYNC_DISCONNECT = 1,

    SYNC_IDLE, //connected

    //master
    SYNC_M_PRECOMMIT_MULTICASTED, //write request from a client, broadcasted precommit
    SYNC_M_COMMITED,              //positive acked by everybody, broadcasted commit and data
    SYNC_M_OPERATION_PERFORMED,   //positive acked by everybody, performed operation

    //slave
    SYNC_S_PRECOMMIT_RECEIVED, //received precommit
    SYNC_S_PRECOMMIT_ACK,      //sent positive ack
    SYNC_S_COMMITED,           //received commit, performed operation, send successful
    SYNC_S_UNDO                //operation unsuccessful, undo

}syncServerState;


int init_sync_server_list();

int start_sync_server();

void *handle_data_sync_event(void *arg);

#endif // SYNC_H_INCLUDED
