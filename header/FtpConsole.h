//
// Created by mars on 11/23/15.
//

#ifndef FTPSERVER_FTPCONSOLE_H
#define FTPSERVER_FTPCONSOLE_H

#include "FtpSession.h"

namespace gnl {

class FtpConsole {

public:
    FtpConsole(FtpSession &session);
    ~FtpConsole(){
        delete (FtpSession*)(&mSession);
    }
    void triggerRead();
    void readLine();

private:
    void parseCommon(char *);
    char* getCommand(char*, char*);


private:
    FtpSession& mSession;

};//end class FtpConsole

}//end namespace gnl


#endif //FTPSERVER_FTPCONSOLE_H
