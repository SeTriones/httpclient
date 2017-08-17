#ifndef BASE_NIO_HANDLER
#define BASE_NIO_HANDLER

#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string.h>
#include <string>

#define MAX_BUFF_LEN    2048
#define READ_BUF_SIZE   1024
#define SOCKET_SND_BUF_SIZE (1024*1024)
#define SOCKET_RCV_BUF_SIZE (1024*1024)


class  base_nio_handler {
    protected:
        int recv_timeout;
        int send_timeout;

    protected:
        int readn_timeout(int fd, std::string &content, int buf_len);
        int readn_timeout(int fd, char* buf, int need_to_read, timeval* timeout);
        int i_read_http_header_timeout(int fd, std::string& header, char* content, int recv_timeout);
        int i_recv_http_res(int fd, char* res_buf, int& res_len, std::string& header, int recv_timeout);
        int read_data(int fd, void * buf, int buf_len, timeval* timeout);
        int writen_timeout(int fd, const void *buf, int buf_len, int to = -1);
        int setSocket(int& fd);
        int connectSocket(sockaddr_in* address, int timeout);
        
};

#endif
