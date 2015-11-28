cmake_minimum_required(VERSION 3.3)
project(gnlFtpServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    header/Defines.h
    header/Epoll.h
    header/EpollException.h
    header/FtpConsole.h
    header/FtpServer.h
    header/FtpSession.h
    header/SocketEception.h
    header/TcpSocket.h
    header/ThreadPool.h
    src/CommonFun.cpp
    src/Epoll.cpp
    src/FtpConsole.cpp
    src/FtpServer.cpp
    src/FtpSession.cpp
    src/main.cpp
    src/TcpSocket.cpp
    src/ThreadPool.cpp)

add_executable(gnlFtpServer ${SOURCE_FILES})