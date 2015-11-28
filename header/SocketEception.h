//
// Created by mars on 11/22/15.
//

#ifndef FTPSERVER_SOCKETECEPTION_H
#define FTPSERVER_SOCKETECEPTION_H
#include <string>
#include <errno.h>
#include <string.h>

namespace gnl {

class SocketException {
public:
    typedef std::string string;

    SocketException(const string &_msg = string())
            : msg(_msg) { }

    string what() const {
        if (errno == 0)
            return msg;
        //如果errno!=0, 则会加上错误描述
        return msg + ": " + strerror(errno);
    }

private:
    string msg;
};

}
#endif //FTPSERVER_SOCKETECEPTION_H
