#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <climits>

#include "common.h"
#include "config.h"
#include "queue.h"
#include "list.h"
#include "msg.h"

using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ios;

int msgNumber = 0;

pthread_mutex_t clientMsgNoLock;

int process_msg(clientInfo *pClient, char *buf, int length, string& response)
{
    std::size_t pos1, pos2;

    string arg;
    string arg1, arg2;
    //string response;

    string msg = string(buf,length);

    string msgSave;
    string username;
    string strNumber;

    fstream myFile;
    string line;

    if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "process_msg: "<< msg << endl;

    //COMMANDS: USER, READ, WRITE, REPLACE, QUIT

    pos1 = msg.find(" ");

    if(pos1 == string::npos)
    {
        //only QUIT command has no space
        if((msg.size()== 6) && ((0 == msg.compare(0,4,"QUIT")) || (0 == msg.compare(0,4,"quit"))))
        {
            response.append("4.0 BYE");
        }

        return 0;
    }

    //message terminated by \r\n or \n
    arg1 = msg.substr(0, pos1);

    if(msg[msg.size()-2] == '\r')
    {
        arg2 = msg.substr(pos1+1, msg.size()-pos1-3); //delete \r\n in the end
    }
    else
    {
        arg2 = msg.substr(pos1+1, msg.size()-pos1-2); //delete \n in the end
    }

    if(arg1.empty() || arg2.empty())
    {
        response.append("ERROR COMMAND");

        return 0;
    }


    if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
    {
        cout << "arg1:" << arg1 << " len:" << arg1.size();
        cout << "   arg2:" << arg2 << " len:" << arg2.size()<<endl;
    }

    //if(0 == arg1.compare(string("USER")))
    if(arg1 == "USER" || arg1 == "user")
    {
        //USER name
        //1.0 HELLO name text
        //1.2 ERROR USER text

        if((arg2.find("/") != string::npos) || (arg2.find(" ") != string::npos))
        {
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
    //else if(0 == arg1.compare(string("READ")))
    else if(arg1 == "READ" || arg1 == "read")
    {
        //READ message-number
        //2.0 MESSAGE message-number poster/message
        //2.1 UNKNOWN message-number text
        //2.2 ERROR READ text
        read_start();

        myFile.open(CONFIG.bbFile, std::ios::in);//read
        if(myFile.is_open())
        {
            while(getline(myFile, line))
            {
                pos2 = line.find("/");
                if(pos2 != std::string::npos)
                {
                    //compare message number
                    if(arg2 == line.substr(0, pos2))
                    {
                        response += "2.0 MESSAGE ";
                        response += line;
                        break;
                    }
                }
            }
            // msg number no found
            if(response.empty())
            {
                response += "2.1 UNKNOWN ";
                response += arg2;
            }

            myFile.close();
        }
        else
        {
            response += "2.2 ERROR READ";
        }

        read_end();
    }
    //else if(0 == arg1.compare(string("WRITE")))
    else if(arg1 == "WRITE" || arg1 == "write")
    {
        //WRITE message
        //save:message-number/poster/message
        //3.0 WROTE message-number
        //3.2 ERROR WRITE text

        read_start();

        myFile.open(CONFIG.bbFile, std::ios::in);//read
        if(myFile.is_open())
        {
            myFile.close();
        }
        else
        {
            response.append("3.2 ERROR WRITE bbfile not exist!");

            return 0;
        }

        read_end();

        msgSaveEvent *pMsgSaveEv = new msgSaveEvent;

        get_new_msg_number(strNumber);

        pMsgSaveEv->msg += strNumber;
        pMsgSaveEv->msg += "/";

        if(strlen(pClient->name) != 0)
        {
            pMsgSaveEv->msg += string(pClient->name, strlen(pClient->name));
        }
        else
        {
            pMsgSaveEv->msg += "nobody";
        }

        //pMsgSaveEv->msg += username;
        pMsgSaveEv->msg += "/";
        pMsgSaveEv->msg += arg2;

        if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout <<"write msg:"<< pMsgSaveEv->msg << endl;

        response.append("3.0 WROTE ");
        response.append(strNumber);

        pMsgSaveEv->event = MSG_SAVE_WRITE;

        enMsgSaveEventQueue(pMsgSaveEv);

        /*
        myFile.open(CONFIG.bbFile, std::ios::app);//write, append
        if(myFile.is_open())
        {
            myFile << msgSave << endl;
            myFile.close();

            response.append("3.0 WROTE ");
            response.append(strNumber);
        }
        */
    }
    //else if(0 == arg1.compare(string("REPLACE")))
    else if(arg1 == "REPLACE" || arg1 == "replace")
    {
        //REPLACE message-number/message
        //3.1 UNKNOWN message-number

        //long posLineStart;
        string msgSaved,userSaved,numberSaved,msgInput,newLine;

        read_start();

        myFile.open(CONFIG.bbFile, std::ios::in);//read
        if(myFile.is_open())
        {
            while(getline(myFile, line))
            {
                //posLineStart = myFile.tellg()-(long)line.length()-(long)1;

                pos1 = line.find_first_of("/");
                pos2 = line.find_last_of("/");

                get_msg_number_byline(numberSaved,line);
                get_msg_username_byline(userSaved, line);
                get_msg_body_byline(msgSaved, line);

                msgInput = arg2.substr(arg2.find("/")+1);

//                if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
//                {
//                    cout << "REPLACE numberSaved:"<<numberSaved<<endl;
//                    cout << "        userSaved:"<<userSaved<<endl;
//                    cout << "        msgSaved:"<<msgSaved<<endl;
//                    cout << "        msgInput:"<<msgInput<<endl;
//                }


                //compare message-number
                if(arg2.substr(0, arg2.find("/")) == line.substr(0, line.find("/")))
                {
                    //cout << "Pos3:"<<std::ios_base::cur << endl;

                    msgSaveEvent *pMsgSaveEvReplace = new msgSaveEvent;

                    pMsgSaveEvReplace->msg = numberSaved;

                    pMsgSaveEvReplace->msg += "/";

                    if(strlen(pClient->name) != 0)
                    {
                        pMsgSaveEvReplace->msg += string(pClient->name, strlen(pClient->name));
                    }
                    else
                    {
                        pMsgSaveEvReplace->msg += "nobody";
                    }

                    pMsgSaveEvReplace->msg += "/";
                    pMsgSaveEvReplace->msg += msgInput;

                    if(msgSaved.length()<= msgInput.length())
                    {
                        pMsgSaveEvReplace->event = MSG_SAVE_REPLACE;

                    }
                    else
                    {
                        //saved message is longer than input
                        pMsgSaveEvReplace->event = MSG_SAVE_REPLACE_PLUS;
                    }

                    enMsgSaveEventQueue(pMsgSaveEvReplace);
/*
                    //replace
                    myFile.seekp(posLineStart);

                    //myFile.write("",line.length());

                    myFile.seekp(posLineStart);

                    myFile << newLine;
*/
                    response.append("3.3 REPLACED ");
                    response.append(arg2);

                    break;
                }
            }

            myFile.close();

            read_end();

            if(response.empty())
            {
                response.append("3.1 UNKNOWN ");
                response.append(arg2);
            }
        }
    }
    //else if(0 == arg1.compare(string("QUIT")))
    else if(arg1 == "QUIT" || arg1 == "quit")
    {
        //QUIT text
        //4.0 BYE text

        response.append("4.0 BYE");
    }
    else
    {
        response.append("ERROR COMMAND");
    }

    return 0;
}


int get_new_msg_number(std::string& strNumber)
{
    int number;

    pthread_mutex_lock(&clientMsgNoLock);

    msgNumber++;

    if(msgNumber == INT_MAX)
    {
        msgNumber = 1;
    }

    number = msgNumber;

    pthread_mutex_unlock(&clientMsgNoLock);

    strNumber += std::to_string(number);

    if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "get new msg number:" << strNumber <<endl;

    return 0;
}

int get_msg_number_byline(std::string& strNumber, std::string& line)
{
    std::size_t pos1;

    pos1 = line.find_first_of("/");

    strNumber = line.substr(0, pos1);

    return 0;
}

int get_msg_username_byline(std::string& user, std::string& line)
{
    std::size_t pos1, pos2;

    pos1 = line.find_first_of("/");
    pos2 = line.find_last_of("/");

    user = line.substr(pos1+1, pos2-pos1-1);

    return 0;
}

int get_msg_body_byline(std::string& msg, std::string& line)
{
    std::size_t pos2;

    pos2 = line.find_last_of("/");

    msg = line.substr(pos2+1, line.length()-pos2-1);

    return 0;
}

int load_msg_number()
{
    string lastline;
    string msgNumLast;

    pthread_mutex_init(&clientMsgNoLock, NULL);

    get_last_line(lastline);

    msgNumLast = lastline.substr(0, lastline.find("/"));

    if(msgNumLast.empty())
    {
        if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
            cout << "can not read last message number!" << endl;

        return -1;
    }

    msgNumber = stoi(msgNumLast);

    if (CONFIG.debugLevel >= DEBUG_LEVEL_APP)
        cout << "load message number:" << msgNumber << endl;

    return 0;
}

int get_time()
{
    time_t now = time(0);

    tm *ltime = localtime(&now);

    cout << 1900 + ltime->tm_year << endl;
    cout << 1 + ltime->tm_mon << endl;
    cout << ltime->tm_mday << endl;
    cout << ltime->tm_hour << ":" << ltime->tm_min << ":" << ltime->tm_sec<<endl;

    return 0;
}



