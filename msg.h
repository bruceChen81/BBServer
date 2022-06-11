#ifndef MSG_H_INCLUDED
#define MSG_H_INCLUDED

#define MSG_GREETING "0.0 greeting COMMANDS: USER, READ, WRITE, REPLACE, QUIT\n"






int process_msg(clientInfo *pClient, char *buf, int length, std::string& response);

int process_sync_msg(clientInfo *pClient, char *buf, int length, std::string& response);

int get_new_msg_number(std::string& strNumber);

int get_msg_number_byline(std::string& strNumber, std::string& line);

int get_msg_username_byline(std::string& user, std::string& line);

int get_msg_body_byline(std::string& msg, std::string& line);

int send_data_sync_event(dataSyncEvType type, std::string& msg);



#endif // MSG_H_INCLUDED
