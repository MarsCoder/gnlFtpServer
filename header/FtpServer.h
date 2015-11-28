//
// Created by mars on 11/23/15.
//

#ifndef FTPSERVER_FTPSERVER_H
#define FTPSERVER_FTPSERVER_H

#include <map>

#include "../header/FtpConsole.h"

namespace gnl {

class FtpServer {

public:
    FtpServer(unsigned short int port = 21, const char *ip = nullptr);

    ~FtpServer() { }

    void start();
    void removeFd(int closeFd);


private:
    unsigned short int mPort;
    const char *mIp;
    std::map<int, FtpConsole&> mConnection;


};//end class FtpServer

}//end namespace gnl


#endif //FTPSERVER_FTPSERVER_H
