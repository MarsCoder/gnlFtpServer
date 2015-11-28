//
// Created by mars on 11/22/15.
//

#ifndef FTPSERVER_EPOLL_H
#define FTPSERVER_EPOLL_H

#include <sys/epoll.h>
#include <vector>
#include <unistd.h>

namespace gnl {

class Epoll {

public:
    Epoll(int flags = EPOLL_CLOEXEC, int noFile = 1024);

    ~Epoll();

    void addfd(int fd, uint32_t events = EPOLLIN, bool ETorNot = false);

    void modfd(int fd, uint32_t events = EPOLLIN, bool ETorNot = false);

    void delfd(int fd);
    int wait(int timeout = -1);

    int getEventOccurfd(int eventIndex) const;

    uint32_t getEvents(int eventIndex) const;

    bool isValid() {
        if (mEpollfd == -1) {
            return false;
        }
        return true;
    }

    void close() {
        if (isValid()) {
            ::close(mEpollfd);
        }
    }

private:
    std::vector<struct epoll_event> events;
    int mEpollfd;
    int mFdNumber;
    int mReady;
};

}

#endif //FTPSERVER_EPOLL_H
