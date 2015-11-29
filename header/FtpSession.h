//
// Created by mars on 11/23/15.
//

#ifndef FTPSERVER_FTPSESSION_H
#define FTPSERVER_FTPSESSION_H

#include "TcpSocket.h"
#include <string>
#include "../header/Defines.h"

namespace gnl {

    enum{
        PASV,
        PORT
    };

class FtpSession {

public:
    FtpSession(int connfd, int clostFd):
            mSocketfd(connfd),
            mCloseFd(clostFd),
            mGreeter("220 gnl Ready!\r\n"),
            mIsLogin(false),
            mDataPort(-1),
            DataMode(PASV){

        if(mRootDirectory.empty()){
            char path[128] = {'\0'};
            getcwd(path, 128);
            mRootDirectory = path;
        }
        mWorkingDirectory = "/";
    }

public:

    FtpResult setUserName(std::string userName);
    FtpResult setUserPassword(std::string userPassword);
    FtpResult setPort(char* data);
    FtpResult setPasv();
    FtpResult getSystem();
    FtpResult sendFileList();
    FtpResult sendFile(std::string);
    FtpResult receiveFile(std::string);
    FtpResult changeWorkingDirectory(std::string);
    FtpResult changeWorkingDirectoryToRoot();
    FtpResult getFileSize(std::string);
    FtpResult quit();
    FtpResult initSession();

    bool isLogin(){return mIsLogin;}

    int getMSocketfd() const {
        return mSocketfd;
    }
    int getMCloseFd() const {
        return mCloseFd;
    }
    const std::string &getMGreeter() const {
        return mGreeter;
    }

    int getMDataPort() const {
        return mDataPort;
    }

    void setMDataPort(int mDataPort) {
        FtpSession::mDataPort = mDataPort;
    }

    const std::string &getMDataAddr() const {
        return mDataAddr;
    }

    void setMDataAddr(const std::string &mDataAddr) {
        FtpSession::mDataAddr = mDataAddr;
    }

    const std::string &getMRootDirectory() const {
        return mRootDirectory;
    }

private:
    bool getFileInfoStat(const char *file_name, char *line,struct stat *s_buff);
    void printOpendirErrorMesg();
    int parsePortData(char* portData, char* clientAddr);
    int connectToClient(TcpSocket&);
    std::string getFullPath(std::string = "");


private:
    int mSocketfd;
    TcpSocket mDataSocket;
    bool mIsLogin;
    int mCloseFd;
    std::string mUserName;
    std::string mUserPassword;
    std::string mRootDirectory;
    std::string mWorkingDirectory;
    std::string mGreeter;

    int mDataPort;
    std::string mDataAddr;
    bool DataMode;

};

}


#endif //FTPSERVER_FTPSESSION_H
