#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#define MSG_GREETING "0.0 GREETING \nCOMMANDS: USER, READ, WRITE, REPLACE, QUIT\nBoth upper and lower case supported, please type \"HELP\" or \"help\" for more information!\n"

#define MSG_HELP "COMMMANS FORMAT:\nUSER name\nREAD message-number\nWRITE message\nREPLACE message-number/message\nQUIT"


int process_client_msg(clientCB *pClient, char *buf, int length, std::string& response);

int process_sync_master_msg(clientCB *pClient, char *buf, int length, std::string& response);

int process_sync_slave_msg(clientCB *pClient, char *buf, int length, std::string& response);



int save_msg_replace(std::string& msgSave);

int save_msg_write(std::string& msgSave);




#endif // MSG_H_INCLUDED
