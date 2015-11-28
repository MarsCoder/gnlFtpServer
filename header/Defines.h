//
// Created by mars on 11/17/15.
//

#ifndef FTPSERVER_DEFINES_H
#define FTPSERVER_DEFINES_H

#define ANON_USER "anonymous"

#define RCVBUFSIZE  512
#define DATABUFSIZE 498

struct FtpResult{
    unsigned short int code;
    std::string message;
    bool operator==(FtpResult result){
        if(code == result.code && !message.compare(result.message)){
            return true;
        }
        return false;
    }
};

//#define REPL_120 "120 Try again in minutes.\r\n"
//#define REPL_125 "125 Data connection alreay open; trasfer string.\r\n"
//#define REPL_150 "150 File status okey; about to open data connection.\r\n"
//#define REPL_200 "200 Command okey.\r\n"
//#define REPL_202 "202 Command not implemented, superfluos at this site.\r\n"
//#define REPL_211 "211 System status, or system help reply.\r\n"
//#define REPL_220 "220 Service ready for new user.\r\n"
//#define REPL_331_ANON "331 Anonymous login ok send your complete email as your passwd\r\n"
//#define REPL_332 "332 Need account for login"

#define REPL_0   FtpResult{0, "none"}
#define REPL_125 FtpResult{125, "Data connection already open; transfer starting."}
#define REPL_150 FtpResult{150, "File status okay; about to open data connection."}
#define REPL_200 FtpResult{200, "Command okay."}
#define REPL_221 FtpResult{221, "closed connecion!"};
#define REPL_226 FtpResult{226, "Closing data connection."}
#define REPL_227 FtpResult{227, "Entering passive mode. "}
#define REPL_425 FtpResult{425, "Can't open data connection."}
#define REPL_451 FtpResult{451, "Requested action aborted. Local error in processing."}
#define REPL_500 FtpResult{500, "Syntax error, command unrecognized."}
#define REPL_501 FtpResult{501, "Syntax error in parameters or arguments."}
#define REPL_502 FtpResult{502, "Command not implemented."}
#define REPL_504 FtpResult{504, "parameter error, Command not implemented."}
#define REPL_530 FtpResult{530, "Not logged in."}
#define REPL_550 FtpResult{550, "Requested action not taken."}

#define THREAD_NUMBER 4

/**设置文件描述符fd为非阻塞/阻塞模式**/
bool setNonBlocking(int fd, bool unBlock);
std::string buildResultMesg( FtpResult result);
bool getFileInfoStat(const char *file_name, char *line,struct stat *s_buff);

#endif //FTPSERVER_DEFINES_H
