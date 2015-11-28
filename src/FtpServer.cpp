//
// Created by mars on 11/23/15.
//

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "../header/FtpServer.h"
#include "../header/FtpConsole.h"
#include "../header/FtpSession.h"
#include "../header/Epoll.h"
#include "../header/ThreadPool.h"
#include "../header/Defines.h"

namespace gnl{

    FtpServer::FtpServer(unsigned short port, const char *ip) :
            mPort(port),mIp(ip) {

    }

    void FtpServer::start() {
        ThreadPool pool(4,1024);
        int closeFd[2];
        setNonBlocking(closeFd[0],true);
        int ret = -1;
        if((ret =pipe(closeFd)) == -1){
            std::cerr << "create closeFd faile!" << std::endl;
            exit(-1);
        }
        bool ee;
        TcpSocket listen(mPort, mIp);
        if(!listen.create()){
            std::cerr << "cerroe!" << std::endl;
            exit(-1);
        }
        if(!listen.setNonBlocking(true)){
            std::cerr << "cerroe!" << std::endl;
            exit(-1);
        }
        if(!listen.reuseAddress()){
            std::cerr << "cerroe!" << std::endl;
            exit(-1);
        }
        if(!listen.bind()){
            std::cerr << "cerroe!" << std::endl;
            exit(-1);
        }
        if(!listen.listen()){
            std::cerr << "cerroe!" << std::endl;
            exit(-1);
        }
        int listenfd = listen.getMSockfd();
        Epoll epoll;
        epoll.addfd(listenfd);
        epoll.addfd(closeFd[0]);
        while(true){
            int ready = epoll.wait();
            for(int i=0; i<ready; ++i) {
                if (epoll.getEventOccurfd(i) == listenfd) {
                    int connfd = accept(listenfd, nullptr, nullptr);
                    if (connfd == -1) {
                        perror("accetp error");
                        exit(-1);
                    }
                    setNonBlocking(connfd, true);
                    epoll.addfd(connfd, EPOLLIN, true);/////
                    FtpSession *ftpSession = new FtpSession(connfd, closeFd[1]);
                    FtpConsole *ftpConsole = new FtpConsole(*ftpSession);
                    mConnection.insert(std::pair<int, FtpConsole &>(connfd, *ftpConsole));
                }else if(epoll.getEventOccurfd(i) == closeFd[0]){
                    removeFd(closeFd[0]);
                }else if(epoll.getEvents(i) & EPOLLIN){
                    int key = epoll.getEventOccurfd(i);
                    FtpConsole &ftpConsole = mConnection.find(key)->second;
                    pool.addTask(std::bind(&FtpConsole::triggerRead, &ftpConsole));
                }
            }
        }
    }

    void FtpServer::removeFd(int closeFd){
        char fd[16] = {'\0'};
        int num = -1;
        if((num = read(closeFd, fd, sizeof(fd)+1)) == -1){
            std::cerr << "read colseFd error!" << std::endl;
            return;
        }else if(num == 0){
            std::cerr << "closeFd[1] has been closed !" << std::endl;
            exit(-1);
        }else{
            int revFd = atoi(fd);
            if(revFd > 2){
                delete (FtpConsole*)&mConnection.find(revFd)->second;
                //delete console;
                mConnection.erase(revFd);   //remove client socket from the mConnection
                close(revFd);   //close client socket
            }
        }
    }
}