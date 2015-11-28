#include <iostream>
#include <signal.h>
#include "../header/TcpSocket.h"
#include "../header/Epoll.h"
#include "../header/FtpServer.h"

using namespace std;

int main() {
    //signal(SIGPIPE, SIG_IGN);
    gnl::FtpServer ftpServer(21);
    ftpServer.start();


    return 0;
}