//
// Created by mars on 11/22/15.
//

#ifndef FTPSERVER_EPOLLEXCEPTION_H
#define FTPSERVER_EPOLLEXCEPTION_H
#include <string>
#include <cstring>
#include <cerrno>
using std::string;
class EpollException
{
public:
    //typedef std::string string;

    EpollException(const string &_msg = "")
        : msg(_msg) {}
    string what() const
    {
        if (errno == 0)
            return msg;
        //如果errno!=0, 则会加上错误描述
        return msg + ": " + strerror(errno);
    }

private:
    string msg;
};
#endif //FTPSERVER_EPOLLEXCEPTION_H
