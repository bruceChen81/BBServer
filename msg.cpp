/*
msg.cpp

Created by Dianyong Chen, 2022-05-28

CS590 Master Project(BBServer) @ Bishop's University

*/

#include "common.h"
#include "config.h"
#include "queue.h"
#include "list.h"
#include "msg.h"
#include "semaphore.h"


int process_sync_master_msg(clientCB *pClient, char *buf, int length, string& response)
{
    string msg, msgnumber;
    syncServerCB *pServer = sync_server_list_find(pClient->fd);

    if(buf[length-2] == '\r'){
        msg += string(buf,length-2);
    }
    else if(buf[length-1] == '\n'){
        msg += string(buf,length-1);
    }
    else{
        msg += string(buf,length);
    }

    LOG(DEBUG_LEVEL_APP)
        cout << "process_sync_master_msg: "<< msg << endl;

    //5.0 PRECOMMIT ACK
    //5.1 ERROR PRECOMMIT

    //6.0 COMMITED SUCCESS message-number
    //6.1 COMMITED UNSUCCESS message-number
    //6.2 COMMITED UNSUCCESS UNKNOWN message-number

    string str1 = "6.0 COMMITED SUCCESS";
    string str2 = "6.1 COMMITED UNSUCCESS";
    string str3 = "6.2 COMMITED UNSUCCESS UNKNOWN";

    if(msg == "5.0 PRECOMMIT ACK"){
        sync_server_list_set_state(pServer, SYNC_M_PRECOMMITED);

        //check if all peers reply ok
        if(sync_check_server_state(SYNC_M_PRECOMMITED)){
            sync_set_master_state(SYNC_M_PRECOMMITED);

            //send precommit ACK message
            sync_send_event(EV_SYNC_PRECOMMIT_ACK, msgnumber, pClient->fd, msg);
        }
    }
    else if(msg == "5.1 ERROR PRECOMMIT"){
        sync_server_list_set_state(pServer, SYNC_M_PRECOMMIT_UNSUCCESS);

        if(sync_get_master_state() != SYNC_M_PRECOMMIT_UNSUCCESS){
            sync_set_master_state(SYNC_M_PRECOMMIT_UNSUCCESS);

            //send precommit err message, only once
            sync_send_event(EV_SYNC_PRECOMMIT_ERR, msgnumber, pClient->fd, msg);
        }
    }
    else if(0 == msg.compare(0, str1.length(), str1))//6.0 COMMITED SUCCESS
    {
        msgnumber = msg.substr(str1.length()+1, msg.length()-str1.length());
        sync_server_list_set_state(pServer, SYNC_M_OPERATION_PERFORMED);

        //check if all peers reply ok
        if(true == sync_check_server_state(SYNC_M_OPERATION_PERFORMED)){
            sync_set_master_state(SYNC_M_OPERATION_PERFORMED);

            //send commit sucess message
            sync_send_event(EV_SYNC_COMMIT_SUCCESS, msgnumber, pClient->fd, msg);
        }
    }
    else if(0 == msg.compare(0, str2.length(), str2)) //6.1 COMMITED UNSUCCESS
    {
        msgnumber = msg.substr(str2.length()+1, msg.length()-str2.length());
        sync_server_list_set_state(pServer, SYNC_M_OPERATION_UNSUCCESS);

        //send commit unsucess message, only once
        if(sync_get_master_state() != SYNC_M_OPERATION_UNSUCCESS){
            sync_set_master_state(SYNC_M_OPERATION_UNSUCCESS);
            sync_send_event(EV_SYNC_COMMIT_UNSUCCESS, msgnumber, pClient->fd, msg);
        }
    }
    else if(0 == msg.compare(0, str3.length(), str3)) //6.2 COMMITED UNSUCCESS UNKNOWN
    {
        msgnumber = msg.substr(str3.length()+1, msg.length()-str3.length());
        sync_server_list_set_state(pServer, SYNC_M_OPERATION_UNSUCCESS);

        //send commit unsucess message, only once
        if(sync_get_master_state() != SYNC_M_OPERATION_UNSUCCESS){
            sync_set_master_state(SYNC_M_OPERATION_UNSUCCESS);
            sync_send_event(EV_SYNC_COMMIT_UNSUCCESS, msgnumber, pClient->fd, msg);
        }
    }
    else{
        cout << "master receive error message" << endl;
    }

    return 0;
}


int process_sync_slave_msg(clientCB *pClient, char *buf, int length, string& response)
{
    string msg, msgnumber;
    fstream myFile;

    if(buf[length-2] == '\r'){
        msg += string(buf,length-2);
    }
    else if(buf[length-1] == '\n'){
        msg += string(buf,length-1);
    }
    else{
        msg += string(buf,length);
    }

    LOG(DEBUG_LEVEL_APP)
        cout << "process_sync_slave_msg: "<< msg << endl;

    //PRECOMMIT, ABORT, COMMIT WRITE/READ message, END SUCCESS, END UNSUCCESS
    string str1 = string("COMMIT WRITE");
    string str2 = string("COMMIT REPLACE");
    string str3 = string("END SUCCESS");
    string str4 = string("END UNSUCCESS");

    if(msg == "PRECOMMIT"){
        sync_set_slave_state(pClient, SYNC_S_PRECOMMIT_RECEIVED);
        response += "5.0 PRECOMMIT ACK";
        sync_set_slave_state(pClient, SYNC_S_PRECOMMIT_ACK);

    }
    else if(msg == "ABORT"){
        sync_set_slave_state(pClient, SYNC_IDLE);
    }
    else if(0 == msg.compare(0, str1.length(), str1)) //COMMIT WRITE
    {
        if(SYNC_S_PRECOMMIT_ACK != pClient->slaveState){
            LOG(DEBUG_LEVEL_D){
                cout << "sync slave recv msg at wrong state";
                print_sync_state(pClient->slaveState);
                cout<< endl;

                return -1;
            }
        }

        string msgSave = msg.substr(str1.length()+1, msg.length()-str1.length());
        msgnumber = msgSave.substr(0, msgSave.find("/"));

        LOG(DEBUG_LEVEL_APP)
            cout << "sync msg write:" <<msgSave<<endl;

        sync_set_slave_state(pClient, SYNC_S_COMMITED);

        update_msg_number(msgnumber);

        if(save_msg_write(msgSave) == 0){
            response.append("6.0 COMMITED SUCCESS");
            //update_msg_number(msgnumber);
        }
        else{
            response.append("6.1 COMMITED UNSUCCESS");
        }

        response += " ";
        response += msgnumber;
    }
    else if(0 == msg.compare(0, str2.length(), str2)) //COMMIT REPLACE
    {
        if(SYNC_S_PRECOMMIT_ACK != pClient->slaveState){
            LOG(DEBUG_LEVEL_D){
                cout << "sync slave recv msg at wrong state";
                print_sync_state(pClient->slaveState);
                cout<< endl;

                return -1;
            }
        }

        string msgReplace = msg.substr(str2.length()+1, msg.length()-str2.length());
        msgnumber = msgReplace.substr(0, msgReplace.find("/"));

        LOG(DEBUG_LEVEL_APP)
            cout <<"sync msg replace:"<<msgReplace<<endl;

        sync_set_slave_state(pClient, SYNC_S_COMMITED);

        //replace
        int ret = save_msg_replace(msgReplace);

        if(ret == 0){
            response.append("6.0 COMMITED SUCCESS");
        }
        else if(ret == -1){
            response.append("6.2 COMMITED UNSUCCESS UNKNOWN");
        }
        else{
            response.append("6.1 COMMITED UNSUCCESS");
        }

        response += " ";
        response += msgnumber;
    }
    else if(0 == msg.compare(0, str3.length(), str3))//END SUCCESS
    {
        msgnumber = msg.substr(str3.length()+1, msg.length()-str3.length());

        sync_set_slave_state(pClient, SYNC_IDLE);
    }
    else if(0 == msg.compare(0, str4.length(), str4))//END UNSUCCESS
    {
        msgnumber = msg.substr(str4.length()+1, msg.length()-str4.length());

        sync_set_slave_state(pClient, SYNC_IDLE);
        //undo

    }


    return 0;

}







int process_client_msg(clientCB *pClient, char *buf, int length, string& response)
{
    std::size_t pos1, pos2;
    string arg, arg1, arg2;
    string line, msgSave, username, strNumber;

    string msg = string(buf,length);

    LOG(DEBUG_LEVEL_APP)
        cout << "process_client_msg: "<< msg << endl;

    //COMMANDS: USER, READ, WRITE, REPLACE, QUIT

    pos1 = msg.find(" ");

    if(pos1 == string::npos){
        //only QUIT and HELP command has no space
        if((msg.size()== 6) && ((0 == msg.compare(0,4,"QUIT")) || (0 == msg.compare(0,4,"quit")))){
            response.append("4.0 BYE");
        }
        else if((msg.size()== 6) && ((0 == msg.compare(0,4,"HELP")) || (0 == msg.compare(0,4,"help")))){
            response.append(MSG_HELP);
        }
        else{
            response.append("ERROR COMMAND");
        }

        return 0;
    }

    //message terminated by \r\n or \n
    arg1 = msg.substr(0, pos1);

    if(msg[msg.size()-2] == '\r'){
        arg2 = msg.substr(pos1+1, msg.size()-pos1-3); //delete \r\n in the end
    }
    else{
        arg2 = msg.substr(pos1+1, msg.size()-pos1-2); //delete \n in the end
    }

    if(arg1.empty() || arg2.empty()){
        response.append("ERROR COMMAND");

        return 0;
    }

    LOG(DEBUG_LEVEL_APP){
        cout << "arg1:" << arg1 << " len:" << arg1.size();
        cout << "   arg2:" << arg2 << " len:" << arg2.size()<<endl;
    }

    if(arg1 == "USER" || arg1 == "user"){
        //USER name
        //1.0 HELLO name text
        //1.2 ERROR USER text

        if((arg2.find("/") != string::npos) || (arg2.find(" ") != string::npos)){
            response.append("1.2 ERROR USER");
        }
        else
        {
            response.append("1.0 HELLO ");
            response.append(arg2);

            //strcpy(pClient->name, arg2.c_str());
            client_list_save_name(pClient->fd, arg2.c_str());
        }
    }
    else if(arg1 == "READ" || arg1 == "read"){
        //READ message-number
        //2.0 MESSAGE message-number poster/message
        //2.1 UNKNOWN message-number text
        //2.2 ERROR READ text

        fstream myFile;

        read_start();

        myFile.open(SysCfgCB.bbFile, std::ios::in);//read
        if(myFile.is_open()){
            while(getline(myFile, line)){
                pos2 = line.find("/");
                if(pos2 != std::string::npos){
                    //compare message number
                    if(arg2 == line.substr(0, pos2)){
                        response += "2.0 MESSAGE ";
                        response += line;
                        break;
                    }
                }
            }
            // msg number no found
            if(response.empty()){
                response += "2.1 UNKNOWN ";
                response += arg2;
            }
            myFile.close();
        }
        else{
            response += "2.2 ERROR READ";
        }

        read_end();
    }
    else if(arg1 == "WRITE" || arg1 == "write"){
        //WRITE message
        //save:message-number/poster/message
        //3.0 WROTE message-number
        //3.2 ERROR WRITE text

        msgSave.clear();
        //get_new_msg_number(strNumber);
        strNumber = alloc_new_msg_number();
        msgSave += strNumber;
        msgSave += "/";

        if(strlen(pClient->name) != 0){
            msgSave += string(pClient->name, strlen(pClient->name));
        }
        else{
            msgSave += "nobody";
        }

        msgSave += "/";
        msgSave += arg2;

        LOG(DEBUG_LEVEL_APP)
            cout <<"write msg:"<< msgSave << endl;

        //check sync peers, if configured, first invoke sync
        if(sync_check_service_enable()){
            //save cmd and msg???
            sync_save_client_cmd(pClient, CLIENT_CMD_WRITE, msgSave);

            if(init_sync_server_connection() < 0){
                response.append("3.2 ERROR WRITE");

                return 0;
            }

            sync_send_precommit();
            return 0;
        }

        //if no sync, write file
        if(save_msg_write(msgSave) == 0){
            response.append("3.0 WROTE ");
            response.append(strNumber);
        }
        else{
            response.append("3.2 ERROR WRITE");
        }
    }
    else if(arg1 == "REPLACE" || arg1 == "replace"){
        //REPLACE message-number/message
        //3.1 UNKNOWN message-number

        string numberInput,msgInput;

        //wrong format
        if(arg2.find("/") == std::string::npos){
            response.append("ERROR COMMAND");
            return 0;
        }

        numberInput = arg2.substr(0, arg2.find("/"));
        msgInput = arg2.substr(arg2.find("/")+1);

        LOG(DEBUG_LEVEL_APP)
            cout <<"replace message body:"<< msgInput.size()<<endl;

        if(msgInput.size() == 0){
            response.append("ERROR COMMAND");
            return 0;
        }

        if(strlen(pClient->name) != 0){
            username += string(pClient->name, strlen(pClient->name));
        }
        else{
            username += "nobody";
        }

        msgSave.clear();
        msgSave += numberInput;
        msgSave += "/";
        msgSave += username;
        msgSave += "/";
        msgSave += msgInput;

        //check sync peers, if configured, first invoke sync
        if(sync_check_service_enable()){
            //save cmd and msg
            sync_save_client_cmd(pClient, CLIENT_CMD_REPLACE, msgSave);

            if(init_sync_server_connection() < 0){
                response.append("3.2 ERROR WRITE");

                return 0;
            }

            sync_send_precommit();

            return 0;
        }

        // if no sync, save
        int ret = save_msg_replace(msgSave);

        if(ret == 0){
            response.append("3.3 REPLACED ");
            response.append(numberInput);
        }
        else if(ret == -1){
            response.append("3.1 UNKNOWN ");
            response.append(numberInput);
        }
        else{
            response.append("3.2 ERROR WRITE");
        }
    }
    else if(arg1 == "QUIT" || arg1 == "quit"){
        //QUIT text
        //4.0 BYE text

        response.append("4.0 BYE");
    }
    else{
        response.append("ERROR COMMAND");
    }

    return 0;
}


//return 0:success; -1: write file error
int save_msg_write(string& msgSave)
{
    LOG(DEBUG_LEVEL_D)
        cout << endl<<"SAVE MSG WRITE:"<< msgSave <<endl;

    fstream myFile;
    int ret = -1;

    write_start();

    myFile.open(SysCfgCB.bbFile, std::ios::app);//write, append

    if(myFile.is_open()){
        myFile << msgSave << endl;
        myFile.close();

        ret = 0;
    }
    else{
        ret = -1;
    }

    write_end();

    return ret;
}

//return 0:success; -1:message-number no found; -2:write file error
int save_msg_replace(string& msg)
{
    LOG(DEBUG_LEVEL_D)
        cout << endl<<"SAVE MSG REPLACE:"<<msg <<endl;

    string line, newLine, numberInput, numberSaved;
    fstream myFile;
    int ret = -1;
    long posLineStart;

    numberInput = msg.substr(0, msg.find("/"));

    write_start();

    myFile.open(SysCfgCB.bbFile, std::ios::in|std::ios::out);//read and write
    if(myFile.is_open()){
        while(getline(myFile, line)){
            posLineStart = myFile.tellg()-(long)line.length()-(long)1;

            numberSaved = line.substr(0, line.find("/"));

            //compare message-number
            if(numberInput == numberSaved){
                //replace
                myFile.seekp(posLineStart);
                myFile << msg;

                if(line.length() > msg.length()){
                    string str = string(line.length() - msg.length(), ' ');
                    myFile << str;
                }

                ret = 0;

                if(SysCfgCB.debugLevel >= DEBUG_LEVEL_APP)
                    cout << "replace msg saved successfully!" << endl;

                break;
            }
        }

        myFile.close();
    }
    else{
        ret = -2;
    }

    write_end();

    return ret;
}





