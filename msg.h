#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#define MSG_GREETING "0.0 greeting COMMANDS: USER, READ, WRITE, REPLACE, QUIT\n"






int process_client_msg(clientInfo *pClient, char *buf, int length, std::string& response);

int process_sync_master_msg(clientInfo *pClient, char *buf, int length, std::string& response);

int process_sync_slave_msg(clientInfo *pClient, char *buf, int length, std::string& response);





int get_new_msg_number(std::string& strNumber);

int get_msg_number_byline(std::string& strNumber, std::string& line);

int get_msg_username_byline(std::string& user, std::string& line);

int get_msg_body_byline(std::string& msg, std::string& line);

int save_msg_replace(std::string& msgSave);

int save_msg_write(std::string& msgSave);









#endif // MSG_H_INCLUDED
