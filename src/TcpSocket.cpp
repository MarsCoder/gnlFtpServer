//
// Created by mars on 11/22/15.
//

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../header/TcpSocket.h"
#include "../header/SocketEception.h"

namespace gnl {

TcpSocket::TcpSocket():mSockfd(-1){
    bzero(&mAddr, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = 0;
    mAddr.sin_addr.s_addr = INADDR_ANY;

}

TcpSocket::TcpSocket(unsigned short int port, const char *ip):mSockfd(-1) {
    bzero(&mAddr, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);
    if(ip == nullptr){
        mAddr.sin_addr.s_addr = INADDR_ANY;
    }else{
        inet_pton(AF_INET, ip, &mAddr.sin_addr);
    }
}

TcpSocket::~TcpSocket() {
    if(isValid())
        ::close(mSockfd);
}

bool TcpSocket::create() {
    if(isValid())
        return false;
    if((mSockfd = ::socket(AF_INET, SOCK_STREAM, 0))== -1)
        return false;
    return true;
}

bool TcpSocket::bind() const {
    if(!isValid())
        return false;

    if(::bind(mSockfd, (const struct sockaddr *)&mAddr, sizeof(mAddr)) == -1){
        return false;
    }
    return true;
}

bool TcpSocket::listen(int backlog) const {
    if(!isValid()){
        return false;
    }
    if(::listen(mSockfd, backlog) == -1){
        return false;
    }
    return true;
}

bool TcpSocket::accept(TcpSocket &clientSocket) const {
    if(!isValid()){
        return false;
    }
    struct sockaddr_in clientaddr;
    socklen_t len;
    clientSocket.setMSockfd(::accept(mSockfd,(sockaddr *)&clientaddr, &len));
    if(clientSocket.getMSockfd() == -1){
        return false;
    }
    clientSocket.setMAddr(clientaddr);
    return true;
}

bool TcpSocket::connect() const {
    if(!isValid()){
        return false;
    }
    if(::connect(mSockfd, (const struct sockaddr *)&mAddr, sizeof(mAddr)) == -1){
        return false;
    }
    return true;
}

bool TcpSocket::setNonBlocking(bool flag) const {
    if(!isValid()){
        return false;
    }
    int opt = fcntl(mSockfd, F_GETFL, 0);
    if(opt == -1){
        return false;
    }
    if(flag){
        opt |= O_NONBLOCK;
    }else{
        opt &= ~O_NONBLOCK;
    }
    if(fcntl(mSockfd, F_SETFL, opt) == -1){
        return false;
    }
    return true;
}

bool TcpSocket::reuseAddress() const {
    if(!isValid()){
        return false;
    }

    int on = 1;
    if(setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
        return false;
    }
    return true;
}

bool TcpSocket::close(){
    if(!isValid()){
        return false;
    }
    ::close(mSockfd);
    mSockfd = -1;
    return true;
}

}