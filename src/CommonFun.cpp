//
// Created by root on 11/25/15.
//

#include <fcntl.h>
#include <string>
#include <sstream>
#include "../header/Defines.h"

/**设置文件描述符fd为非阻塞/阻塞模式**/
bool setNonBlocking(int fd, bool unBlock)
{
    int flags = fcntl(fd,F_GETFL);
    if (flags == -1)
        return false;

    if (unBlock)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd,F_SETFL,flags) == -1)
        return false;
    return true;
}

std::string buildResultMesg( FtpResult result) {
    std::stringstream stream;
    stream << result.code << " " << result.message <<"\r\n";
    return stream.str();
}
