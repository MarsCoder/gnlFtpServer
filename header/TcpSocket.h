//
// Created by mars on 11/22/15.
//

#ifndef FTPSERVER_TCPSOCKET_H
#define FTPSERVER_TCPSOCKET_H

#include <netinet/in.h>


namespace gnl {

class TcpSocket {
public:
    TcpSocket();
    TcpSocket(unsigned short int port, const char *ip = nullptr);

    ~TcpSocket();

    bool create();

    bool bind() const;

    bool listen(int backlog = 5) const;

    bool accept(TcpSocket &clientSocket) const;

    bool connect() const;

    bool reuseAddress() const;

    bool isValid() const {
        return (mSockfd != -1);
    }

    bool setNonBlocking(bool flag) const;

    bool close();

private:
public:
    int getMSockfd() const { return mSockfd; }

    void setMSockfd(int mSockfd) { TcpSocket::mSockfd = mSockfd; }

    struct sockaddr_in &getMAddr() { return mAddr; }

    void setMAddr(const sockaddr_in &mAddr) { TcpSocket::mAddr = mAddr; }

private:
    int mSockfd;
    struct sockaddr_in mAddr;


};

}


#endif //FTPSERVER_TCPSOCKET_H
