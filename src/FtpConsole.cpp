//
// Created by mars on 11/23/15.
//

#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include "../header/FtpConsole.h"

namespace gnl{


    FtpConsole::FtpConsole(FtpSession &session):mSession(session)
    {

        write(mSession.getMSocketfd(), mSession.getMGreeter().c_str(), mSession.getMGreeter().size());
    }

    void FtpConsole::triggerRead(){
        char buf[256] = {'\0'};
        int ret = -1;
        if((ret = read(mSession.getMSocketfd(),buf, sizeof(buf))) == -1){
            std::cerr << "read error!" << std::endl;
        }else if(ret == 0){
            char fd[16] = {'\0'};
            snprintf(fd, sizeof(int)+1, "%d", mSession.getMSocketfd());
            //////////////lock ???/////////////////////////////////////////////////////
            write(mSession.getMCloseFd(), fd, sizeof(int)+1);
            std::cout << mSession.getMSocketfd() << " close connection!" << std::endl;
        }else{
            parseCommon(buf);
        }
    }

    void FtpConsole::parseCommon(char *buf) {

        char command[16] = {'\0'};
        char *data =  getCommand(buf, command);

        FtpResult result;
        if(strcmp(command,"USER") == 0) {
            result = mSession.setUserName(data);
        } else if (strcmp(command, "PASS") == 0) {
            result = mSession.setUserPassword(data);
        } else if(strcmp(command, "ACCT") == 0){
            result = mSession.setUserName(data);
        } else if (strcmp(command, "SYST") == 0) {		//查找服务器的操作系统类型
            result = mSession.getSystem();
        } else if (strcmp(command,"PWD") == 0){	//回到父目录
            result = mSession.changeWorkingDirectory("/");
        } else if (strcmp(command, "TYPE") == 0) {		//参数指定表示类型
            result = REPL_200;
        } else if (strcmp(command, "PORT") == 0) {        //
            result = mSession.setPort(data);
        } else if (strcmp(command, "PASV") == 0){
            result = mSession.setPasv();
            if(result == REPL_0)
                return;
        } else if (strcmp(command, "SIZE") == 0) {		//文件大小
            result = mSession.getFileSize(data);
        } else if (strcmp(command, "LIST") == 0) {		//列表
            result = mSession.sendFileList();
        } else if (strcmp(command, "RETR") == 0) {	//send file to client
            result = mSession.sendFile(data);
        } else if (strcmp(command, "STOR") == 0) {	//receive file from client
            result = mSession.receiveFile(data);
        } else if (strcmp(command, "CWD") == 0){	//改变工作路径
            result = mSession.changeWorkingDirectory(data);
        } else if(strcmp(command, "QUIT") == 0) {
            result = mSession.quit();
        } else if(strcmp(command, "REIN") == 0){
            result = mSession.initSession();
        } else if(strcmp(command, "MODE") == 0) {
            result = REPL_504;
        } else if(strcmp(command, "STRU")){
            result =  REPL_504;
        } else if(strcmp(command, "NOOP") == 0){
            result =  REPL_200;
        } else {
            result = REPL_500;
        }
        std::string mesg = buildResultMesg(result);
        write(mSession.getMSocketfd(), mesg.c_str(), mesg.size());
    }

    char* FtpConsole::getCommand(char* buf, char* command){
        while(*buf == ' '){
            ++buf;
        }
        int i = 0,j = 0;
        while(*buf != ' ' && *buf != '\r' && *buf != '\n' && *buf != '\0'){
            command[i] = *buf++;
            ++i;
        }
        command[i] = '\0';
        while(*buf == ' '){
            ++buf;
        }
        char* data = buf;
        i = 0;
        while(buf[i] != '\r' && buf[i] != '\n'){
            ++i;
        }
        buf[i] = '\0';
        return data;
    }


}
