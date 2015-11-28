//
// Created by mars on 11/22/15.
//

#include "../header/Epoll.h"
#include "../header/EpollException.h"
#include <sys/resource.h>

namespace gnl{

Epoll::Epoll(int flags, int noFile):mFdNumber(0),mReady(0)
{
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = noFile;
    if(::setrlimit(RLIMIT_NOFILE, &rlim) == -1)
        throw EpollException("setrlimit error");
    if((mEpollfd = ::epoll_create(flags)) == -1){
        throw EpollException("epoll_create error");
    }
}

Epoll::~Epoll() {
    this->close();
}

void Epoll::addfd(int fd, uint32_t events, bool ETorNot)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = events;
    if(ETorNot)
        event.events |= EPOLLET;
    event.data.fd = fd;
    if(::epoll_ctl(mEpollfd, EPOLL_CTL_ADD, fd, &event) == -1)
        throw EpollException("epoll_ctl_add error");
    ++mFdNumber;
}

void Epoll::modfd(int fd, uint32_t events, bool ETorNot)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = events;
    if(ETorNot){
        event.events |= EPOLLIN;
    }
    event.data.fd = fd;
    if(::epoll_ctl(mEpollfd, EPOLL_CTL_MOD, fd, &event) == -1)
        throw EpollException("epoll_ctl_mod error");
}

void Epoll::delfd(int fd)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    if(::epoll_ctl(mEpollfd, EPOLL_CTL_DEL, fd, &event) == -1)
        throw EpollException("epoll_ctl_del error");
    --mFdNumber;
}

int Epoll::wait(int timeout)
{
    events.resize(mFdNumber);
    while(true){
        mReady = epoll_wait(mEpollfd,&*events.begin(), mFdNumber,timeout);
        if(mReady == 0)
            throw EpollException("epoll_wait timeout");
        else if(mReady == -1){
            if(errno == EINTR)
                continue;
            else
                throw EpollException("epoll_wait error");
        }else{
            return mReady;
        }
    }
    return -1;
}

int Epoll::getEventOccurfd(int eventIndex) const {
    if(eventIndex > mReady)
        throw EpollException("parameter error");
    return events[eventIndex].data.fd;
}

uint32_t Epoll::getEvents(int eventIndex) const {
    if(eventIndex > mReady)
        throw EpollException("parameter reeor");
    return events[eventIndex].events;
}

}//end namespace gnl
