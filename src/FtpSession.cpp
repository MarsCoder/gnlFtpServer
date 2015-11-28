//
// Created by mars on 11/23/15.
//

#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <pwd.h>
#include <grp.h>
#include "../header/FtpSession.h"
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <algorithm>
#include <sstream>
#include "../header/SocketEception.h"

namespace gnl{

    FtpResult FtpSession::setUserName(std::string userName)
    {
        mUserName = userName;
        return FtpResult{331, "Please enter your password:"};
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::setUserPassword(std::string userPassword)
    {
        if(mUserName.compare("anonymous") == 0){
            mIsLogin = true;
            return FtpResult{230, "Login successful."};
        }
        return FtpResult{ 332, "Login failed! Please try again" };
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::getSystem()
    {
        return FtpResult{215, "Windows"};
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::sendFileList()
    {
        if(!isLogin()){
            return REPL_501;
        }
        int fd = -1;
        TcpSocket client(mDataPort, mDataAddr.c_str());
        if((fd = connectToClient(client)) == -1){
            return REPL_425;
        }
        std::string mesg = buildResultMesg(REPL_150);
        write(mSocketfd, mesg.c_str(), mesg.size());

        struct dirent *p_dirent = NULL;
        char line[256] = {'\0'};
        DIR *dir = opendir(mWorkingDirectory.c_str());

        if(dir==NULL) {
            close(fd);
            closedir(dir);
            printOpendirErrorMesg();
            return REPL_451;
        }
        while((p_dirent = readdir(dir)) != NULL){
            struct stat s_buff;
            std::string fileName = mWorkingDirectory;
            fileName.append("/").append(p_dirent->d_name);
            int status = stat(fileName.c_str(),&s_buff);
            if(status==0) {
                getFileInfoStat(p_dirent->d_name,line,&s_buff);
                if(write(fd, line, strlen(line)) == -1){
                    close(fd);
                    closedir(dir);
                    return REPL_451;
                }
            }
        }
        close(fd);
        closedir(dir);
        return REPL_226;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::changeWorkingDirectory(std::string directory)
    {
        if(!isLogin()){
            return REPL_530;
        }
        if(directory.empty()){
            return FtpResult{257, mWorkingDirectory};
        }
        if(directory.size() > 1 && directory.back() == '/'){//  dir/
            directory.pop_back();
        }
        if(directory.size()>4 && directory.compare(0, 5, "../..") == 0){
            return REPL_501;
        }
        if(directory.compare(".") == 0){
            return FtpResult{257, mWorkingDirectory};
        }else if(directory.compare("..") == 0){
            if(mWorkingDirectory.compare(mRootDirectory) == 0){
                return FtpResult{257, mWorkingDirectory};
            }
            int n = mWorkingDirectory.find_last_of('/');
            if(n == 0) {
                mWorkingDirectory.resize(1);
            }
            else {
                mWorkingDirectory.resize(n);
            }
            return FtpResult{257, mWorkingDirectory};
        }else if(directory.compare("/") == 0){
            mWorkingDirectory = mRootDirectory;
            return FtpResult{257, mWorkingDirectory};
        }else if(directory.at(0) == '/'){
            struct stat s_buff;
            std::string dir = mRootDirectory+directory;
            int status = stat(dir.c_str(),&s_buff);
            if(status != 0){
                return REPL_451;
            }
            if(S_ISDIR(s_buff.st_mode)) {
                mWorkingDirectory = mRootDirectory+directory;
                return FtpResult{257, mWorkingDirectory};
            }
            return FtpResult{501, directory};
        }
        std::string fileName = mWorkingDirectory;
        if(mWorkingDirectory.size() > 1) {
            fileName.append("/").append(directory);
        }else{
            fileName.append(directory);
        }
        struct stat s_buff;
        int status = stat(fileName.c_str(),&s_buff);
        if(status != 0){
            return REPL_451;
        }
        if(S_ISDIR(s_buff.st_mode)) {
            if(mWorkingDirectory.size() > 1) {
                mWorkingDirectory.append("/").append(directory);
            }else{
                mWorkingDirectory.append(directory);
            }
            return FtpResult{257, mWorkingDirectory};
        }
        return FtpResult{501, directory +  " is not a directory."};
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::setPort(char* data){
        if(isLogin()){
            char clientAddr[16] = {'\0'};
            int port = -1;
            port = parsePortData(data,clientAddr);
            if(port > 0) {
                DataMode = PORT;
                setMDataPort(port);
                setMDataAddr(clientAddr);
                return REPL_200;
            }else{
                return REPL_501;
            }
        }else{
            return REPL_530;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::setPasv()
    {
        if(!isLogin()){
            return REPL_530;
        }
        TcpSocket client;
        if(!client.create()){
            std::cerr << "create new socket error! mesg:" << strerror(errno) << std::endl;
            return REPL_502;
        }
        if(!client.bind()){
            client.close();
            return REPL_502;
        }
        if(!client.listen()){
            client.close();
            return REPL_502;
        }
        char ip[16] = {'\0'};
        unsigned short int port = -1;
        port = client.getMAddr().sin_port;
        struct sockaddr_in connAddr;
        socklen_t len = sizeof(connAddr);
        int ret = getsockname(client.getMSockfd(), (struct sockaddr*)&connAddr, &len);
        if (0 != ret){
            client.close();
            return REPL_502;
         }
        port = ntohs(connAddr.sin_port); // 获取端口号
        ret = getsockname(mSocketfd, (struct sockaddr*)&connAddr, &len);
        if (0 != ret){
            client.close();
            return REPL_502;
        }
        inet_ntop(AF_INET, &connAddr.sin_addr, ip, 16);
        std::string mesg("Entering passive mode (");
        mesg.append(ip);
        std::replace(mesg.begin(), mesg.end(), '.', ',');
        std::stringstream hPort;
        std::stringstream lPort;
        hPort << port/256;
        lPort << (port&0x0ff);
        mesg.append(",").append(hPort.str()).append(",").append(lPort.str()).append(")");
        mesg = buildResultMesg(FtpResult{227, mesg});
        write(mSocketfd, mesg.c_str(), mesg.size());
        DataMode = PASV;
        if(!client.accept(mDataSocket)){
            client.close();
            return REPL_502;
        }
        client.close();
        return REPL_0;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::sendFile(std::string fileName)
    {
        if(!isLogin()){
            return REPL_501;
        }
        if(fileName.at(0) == '/'){
            fileName = mRootDirectory + fileName;
        }else{
            fileName = mWorkingDirectory + "/" + fileName;
        }
        struct stat s_buff;
        int status = stat(fileName.c_str(),&s_buff);
        if(status!=0 ){//|| S_ISDIR(s_buff.st_mode)) {
            return REPL_501;
        }
        int dataFd = -1;
        TcpSocket client(mDataPort, mDataAddr.c_str());
        if((dataFd = connectToClient(client)) == -1){
            return REPL_425;
        }
        std::string mesg = buildResultMesg(REPL_150);
        if(write(mSocketfd, mesg.c_str(), mesg.size()) == -1){
            close(dataFd);
            return REPL_451;
        }
        int fd = open(fileName.c_str(),O_RDONLY);
        if(fd == -1){
            close(dataFd);
            close(fd);
            return REPL_501;
        }
        unsigned long filesize = -1;
        filesize = s_buff.st_size;
        if(sendfile(dataFd,fd,nullptr,filesize) == -1){
            close(dataFd);
            close(fd);
            return REPL_451;
        }
        close(dataFd);
        close(fd);
        return REPL_226;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::receiveFile(std::string fileName)
    {
        if(!isLogin()){
            return REPL_501;
        }
        if(fileName.at(0) == '/'){
            fileName = mRootDirectory + fileName;
        }else{
            fileName = mWorkingDirectory + "/" + fileName;
        }
        int dataFd = -1;
        TcpSocket client(mDataPort, mDataAddr.c_str());
        if((dataFd = connectToClient(client)) == -1){
            return REPL_425;
        }
        std::string mesg = buildResultMesg(REPL_150);
        write(mSocketfd, mesg.c_str(), mesg.size());
        int fd = open(fileName.c_str(),O_WRONLY|O_CREAT);
        if(fd == -1){
            close(dataFd);
            return REPL_451;
        }
        int pip[2];
        int ret = pipe(pip);
        if(ret == -1){
            close(dataFd);
            close(fd);
            return REPL_451;
        }

        splice(dataFd, nullptr, pip[1], nullptr, 10240, SPLICE_F_MOVE);
        splice(pip[0], nullptr, fd, nullptr, 10240, SPLICE_F_MOVE);
        close(dataFd);
        close(pip[0]);
        close(pip[1]);
        close(fd);
        return REPL_226;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::changeWorkingDirectoryToRoot()
    {
        mWorkingDirectory = mRootDirectory;
        return FtpResult{257, mWorkingDirectory};
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::getFileSize(std::string fileName)
    {
        struct stat s_buff;
        int status = stat(fileName.c_str(),&s_buff);
        if(status!=0) {
            return REPL_501;
        }
        std::stringstream size;
        size << s_buff.st_size;
        return FtpResult{213, size.str()};

    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::quit()
    {
        char fd[16] = {'\0'};
        snprintf(fd, sizeof(int)+1, "%d", mSocketfd);
        //////////////lock ???/////////////////////////////////////////////////////
        write(mCloseFd, fd, sizeof(int)+1);
        std::cout << mSocketfd << " close connection!" << std::endl;
        return REPL_221;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    FtpResult FtpSession::initSession()
    {
        return REPL_502;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool FtpSession::getFileInfoStat(const char *file_name, char *line,struct stat *s_buff){
        char date[16];
        char mode[11]	= "----------";
        line[0]='\0';
        struct passwd * pass_info = getpwuid(s_buff->st_uid);
        if(pass_info!=NULL) {
            struct group * group_info = getgrgid(s_buff->st_gid);
            if(group_info!=NULL) {
                int b_mask = s_buff->st_mode & S_IFMT;
                if(b_mask == S_IFDIR) {
                    mode[0]='d';
                } else if(b_mask == S_IFREG){
                    mode[0]='-';
                } else {
                    return false;
                }
                mode[1] = (s_buff->st_mode & S_IRUSR)?'r':'-';
                mode[2] = (s_buff->st_mode & S_IWUSR)?'w':'-';
                mode[3] = (s_buff->st_mode & S_IXUSR)?'x':'-';
                mode[4] = (s_buff->st_mode & S_IRGRP)?'r':'-';
                mode[5] = (s_buff->st_mode & S_IWGRP)?'w':'-';
                mode[6] = (s_buff->st_mode & S_IXGRP)?'x':'-';
                mode[7] = (s_buff->st_mode & S_IROTH)?'r':'-';
                mode[8] = (s_buff->st_mode & S_IWOTH)?'w':'-';
                mode[9] = (s_buff->st_mode & S_IXOTH)?'x':'-';
                strftime(date,13,"%b %d %H:%M",localtime(&(s_buff->st_mtime)));
                sprintf(line,"%s %3d %-4s %-4s %8d %12s %s\r\n",mode,s_buff->st_nlink,pass_info->pw_name,group_info->gr_name,s_buff->st_size,date,file_name);
                return true;
            }
        }
        return false;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int FtpSession::parsePortData(char* portData, char* clientAddr)
    {
        clientAddr[0]='\0';
        int len=0;
        int port=0;
        int toint=0;
        char *result;
        result = strtok(portData, ",");

        toint=atoi(result);
        if(toint<1 || toint>254)
            return -1;
        len += strlen(result);
        strcpy(clientAddr,result);
        clientAddr[len]='\0';
        strcat(clientAddr,".");
        len++;

        result = strtok(NULL, ",");
        toint = atoi(result);
        if(toint<0 || toint>254)
            return -1;
        len += strlen(result);
        strcat(clientAddr,result);
        clientAddr[len]='\0';
        strcat(clientAddr,".");
        len++;

        result = strtok(NULL, ",");
        toint = atoi(result);
        if(toint<0 || toint>254)
            return -1;
        len += strlen(result);
        strcat(clientAddr,result);
        clientAddr[len]='\0';
        strcat(clientAddr,".");
        len++;

        result = strtok(NULL, ",");
        toint = atoi(result);
        if(toint<0 || toint>254)
            return -1;
        len += strlen(result);
        strcat(clientAddr,result);
        clientAddr[len]='\0';

        result = strtok(NULL, ",");
        toint = atoi(result);
        if(toint<0 || toint>254)
            return -1;
        port = 256*toint;
        result = strtok(NULL, ",");
        toint = atoi(result);
        if(toint<0 || toint>254)
            return -1;
        port +=toint;
        return port;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    void FtpSession::printOpendirErrorMesg(){
        printf("Error openning directory \"%s\", error was:\n  ",mWorkingDirectory.c_str());
        switch(errno) {
            case EACCES:
                printf("Access denied.\n");
                break;
            case EMFILE:
                printf("Too many file descriptors in use by process.\n");
                break;
            case ENFILE:
                printf("Too many files are currently open in the system.\n");
                break;
            case ENOENT:
                printf("Directory does not exist, or is an empty string.\n");
                break;
            case ENOMEM:
                printf("Insufficient memory to complete the operation..\n");
                break ;
            case ENOTDIR:
            default:
                printf("\"%s\" is not a directory.\n",mWorkingDirectory.c_str());
                break ;
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    int FtpSession::connectToClient(TcpSocket &client)
    {
        if(DataMode == PASV) {
            return mDataSocket.getMSockfd();
        }else{
            if(!client.create()){
                std::cerr << "create new socket error! mesg:" << strerror(errno) << std::endl;
                return false;
            }
            if (!client.connect()) {
                std::cerr << "connect to data connetion error! mesg:" << strerror(errno) << std::endl;
                client.close();
                return false;
            }
            return client.getMSockfd();
        }
    }
}