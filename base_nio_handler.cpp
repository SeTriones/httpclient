#include "base_nio_handler.hpp"
#include "service_log.hpp"
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int base_nio_handler::readn_timeout(int fd, std::string &content, int buf_len) {
    char buf[READ_BUF_SIZE+1];
    timeval timeout = {
        0,
        recv_timeout * 1000
    };
    int n,len;
    int left = buf_len;

    while (left > 0)
    {
        len = left>READ_BUF_SIZE?READ_BUF_SIZE:left;
        if ((n = read_data(fd, buf, len,&timeout)) <= 0)
            return buf_len - left;

        buf[n] = '\0';
        content += buf;
        left -= n;
    }

    return buf_len;
}

int base_nio_handler::readn_timeout(int fd, char* content, int need_to_read, timeval* timeout) {
	char buf[READ_BUF_SIZE + 1];
	int n, left;
	int len;
    int ptr = 0;
	left = need_to_read;
	while (left > 0) {
		len = left > READ_BUF_SIZE ? READ_BUF_SIZE : left;
		n = read_data(fd, buf, len, timeout);
		if (n <= 0 ) {
			if(n == 0)
				return -2;
			return need_to_read - left;
		}
		buf[n] = '\0';
        memcpy(content + ptr, buf, len);
        ptr = ptr + n;
		left = left - n;
	}
	return need_to_read;
}

/*
int base_nio_handler::read_data(int fd, void * buf, int buf_len, timeval* timeout) {
    pollfd read_fd;
    read_fd.fd = fd;
    read_fd.events = POLLIN;
    int poll_ret = poll(&read_fd, 1, timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    if (poll <= 0 || !(read_fd.revents & POLLIN)) {
        return -1; 
    }
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    if (select(fd + 1, &rset, NULL, NULL, timeout) <= 0) {
        //_INFO("select errno=%d", errno);
        return -1;
    }
    */
//    return read(fd, buf, buf_len);
//}

int base_nio_handler::read_data(int fd, void * buf, int buf_len, timeval* timeout) {
    
    if (!(timeout->tv_sec >= 0 && timeout->tv_usec >=0))
        return -1;    

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
        
    pollfd read_fd;
    read_fd.fd = fd;
    read_fd.events = POLLIN;
    int poll_ret = poll(&read_fd, 1, timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
    if (poll_ret <= 0 || !(read_fd.revents & POLLIN)) {
        fprintf(stderr, "rd ret=%d, errno=%d\n", poll_ret, errno);
        if (read_fd.revents & POLLIN) {
            fprintf(stderr, "POLLIN\n");
        }
        if (read_fd.revents & POLLPRI) {
            fprintf(stderr, "POLLPRI\n");
        }
        if (read_fd.revents & POLLOUT) {
            fprintf(stderr, "POLLOUT\n");
        }
        if (read_fd.revents & POLLRDHUP) {
            fprintf(stderr, "POLLRDHUP\n");
        }
        if (read_fd.revents & POLLERR) {
            fprintf(stderr, "POLLERR\n");
        }
        if (read_fd.revents & POLLHUP) {
            fprintf(stderr, "POLLHUP\n");
        }
        if (read_fd.revents & POLLNVAL) {
            fprintf(stderr, "POLLNVAL\n");
        }
        if (read_fd.revents & POLLRDNORM) {
            fprintf(stderr, "POLLRDNORM\n");
        }
        if (read_fd.revents & POLLRDBAND) {
            fprintf(stderr, "POLLRDBAND\n");
        }
        if (read_fd.revents & POLLWRNORM) {
            fprintf(stderr, "POLLWRNORM\n");
        }
        if (read_fd.revents & POLLWRBAND) {
            fprintf(stderr, "POLLWRBAND\n");
        }
        return -1; 
    }
    int len = read(fd,buf,buf_len);
    gettimeofday(&end_time,NULL);
    long cost = 1000000 * (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec);
    if (timeout->tv_sec > 0) {
        timeout->tv_usec += timeout->tv_sec * 1000000;
        timeout->tv_sec = 0;
    }
    timeout->tv_usec -= cost; 
    //fprintf(stderr,"ori tv_sec=%ld,tv_usec=%ld,cost=%ld\n",timeout->tv_sec,timeout->tv_usec,cost);
    if (len == 0) {
        _INFO("read_len=0,disconnect by peer");
    }
    return len;
}

int base_nio_handler::writen_timeout(int fd, const void *buf, int buf_len, int to) {
	int left = buf_len;
	int n;
    //fd_set wset;
    pollfd write_fd;
    write_fd.fd = fd;
    write_fd.events = POLLOUT;
    int poll_ret;

	int real_to = to == -1 ? send_timeout : to;
    timeval timeout = {
        0,
        real_to * 1000
    };

    while (left > 0) {
        poll_ret = poll(&write_fd, 1, timeout.tv_sec * 1000 + timeout.tv_usec / 1000);
        if (poll_ret <= 0 || !(write_fd.revents & POLLOUT)) {
            fprintf(stderr, "wr ret=%d, errno=%d\n", poll_ret, errno);
            if (write_fd.revents & POLLIN) {
                fprintf(stderr, "POLLIN\n");
            }
            if (write_fd.revents & POLLPRI) {
                fprintf(stderr, "POLLPRI\n");
            }
            if (write_fd.revents & POLLOUT) {
                fprintf(stderr, "POLLOUT\n");
            }
            if (write_fd.revents & POLLRDHUP) {
                fprintf(stderr, "POLLRDHUP\n");
            }
            if (write_fd.revents & POLLERR) {
                fprintf(stderr, "POLLERR\n");
            }
            if (write_fd.revents & POLLHUP) {
                fprintf(stderr, "POLLHUP\n");
            }
            if (write_fd.revents & POLLNVAL) {
                fprintf(stderr, "POLLNVAL\n");
            }
            if (write_fd.revents & POLLRDNORM) {
                fprintf(stderr, "POLLRDNORM\n");
            }
            if (write_fd.revents & POLLRDBAND) {
                fprintf(stderr, "POLLRDBAND\n");
            }
            if (write_fd.revents & POLLWRNORM) {
                fprintf(stderr, "POLLWRNORM\n");
            }
            if (write_fd.revents & POLLWRBAND) {
                fprintf(stderr, "POLLWRBAND\n");
            }
            return -1;
        }
        /*
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        if (select(fd + 1, NULL, &wset, NULL, &timeout) <= 0)
            return -1;
        */
		if ((n = write(fd, buf, left)) <= 0)
			return buf_len - left;

		buf = (char *)buf + n;
		left -= n;
    }
    return buf_len;
}

int base_nio_handler::setSocket(int& fd) {
    if (fd <= 0) {
        _ERROR("error socket fd=%d", fd);
        return -1;
    }
    int options;
    options = SOCKET_SND_BUF_SIZE;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &options, sizeof(int));
    options = SOCKET_RCV_BUF_SIZE;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &options, sizeof(int));
    options = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
    options = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, options | O_NONBLOCK);
    int on = 1;
    int ret = -1;
    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
    return ret;
}

int base_nio_handler::connectSocket(sockaddr_in* address, int timeout) {
	int fd = -1;
	if (address == NULL) {
		_ERROR("null address");
		return -1;
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		_ERROR("create socket fd error on %s:%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
		return -1;
	}
	int options;
	options = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, options | O_NONBLOCK);
	timeval begin;
	gettimeofday(&begin, NULL);
	if (connect(fd, (struct sockaddr*)address, sizeof(*address)) < 0) {
		if (errno == EINPROGRESS) {
			pollfd connect_fd;
			connect_fd.fd = fd;
			connect_fd.events = POLLOUT;
			int poll_ret = poll(&connect_fd, 1, timeout);
			timeval end;
			gettimeofday(&end, NULL);
			_INFO("connect cost=%ld", end.tv_sec * 1000000 + end.tv_usec - begin.tv_sec * 1000000 - begin.tv_usec);
			if (poll_ret <= 0 || !(connect_fd.revents & POLLOUT)) {
				_ERROR("connect socket fail on %s:%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port));
				close(fd);
				return -1;
			}
			int opt;
			socklen_t  olen = sizeof(opt);
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &opt, &olen) != 0) {
				_ERROR("get new fd opt err");
				close(fd);
				return -1;
			}
			if (opt != 0) {
				_ERROR("connect socket fail2 on %s:%d, err=%d", inet_ntoa(address->sin_addr), ntohs(address->sin_port), opt);
				close(fd);
				return -1;
			}
		}
		else {
			close(fd);
			return -1;
		}
	}

	setSocket(fd);
    return fd;
}

int base_nio_handler::i_read_http_header_timeout(int fd, std::string& header, char* content, int recv_timeout) {
    char buf[READ_BUF_SIZE + 1];
	buf[READ_BUF_SIZE] = 0;
    int n;
    size_t pos;
    std::string pkg;
    timeval timeout = {
        0,
        recv_timeout * 1000
    };
    timeval begin;
    gettimeofday(&begin, NULL);
    int total_len = 0;
    int last_n = 0;

    while (1) {
        pos = pkg.find("\r\n\r\n");
        if (pos != std::string::npos) {
            header = pkg.substr(0, pos + 4);
            int header_end = pos + 4;
            pos = header_end - (total_len - last_n);
            if (last_n - pos > 0) {
                memcpy(content, buf + pos, last_n - pos);
            }
            return last_n - pos;
        }
        n = read_data(fd, buf, READ_BUF_SIZE, &timeout);
        if (n <= 0) {
            _ERROR("in recv http header errno=%d, n=%d", errno, n);
			if(n == 0)
				return -2;
            return -1;
        }
        buf[n] = '\0';
        pkg += buf;
        last_n = n;
        total_len += n;
    }

    return -1;
}

int base_nio_handler::i_recv_http_res(int fd, char* res_buf, int& res_len, std::string& header, int recv_timeout) {
    const int max_res_buf_len = res_len;
	header = "";
    res_len = 0;
    int ret = i_read_http_header_timeout(fd, header, res_buf, recv_timeout);
    if (ret > max_res_buf_len) {
        _ERROR("body received with header exceed %d", max_res_buf_len);
        return -1;
    }
    if (ret < 0) {
        _ERROR("receive header error");
		if(ret == -2)
			return -5;
        return -1;
    }
    // ÓÐcontent-lenµÄ
    size_t len_idx = header.find("Content-Length:");
    if (len_idx != std::string::npos) {
        //_INFO("sugg header with length");
        timeval timeout = {
            0,
            recv_timeout * 1000
        };
        int sep_index = header.find("\r\n", len_idx);
        int body_len = atoi(header.substr(len_idx + strlen("Content-Length:"), sep_index - len_idx - strlen("Content-Length:")).c_str());
        if (body_len > max_res_buf_len) { 
            _ERROR("body len=%d, exceed max_len=%d", body_len, max_res_buf_len);
        }
        int need_to_read = body_len - ret;
		if (need_to_read < 0) {
			_ERROR("sugg need_to_read err=%d", need_to_read);
			return -4;
		}
        ret = readn_timeout(fd, res_buf + ret, need_to_read, &timeout);
        if (ret != need_to_read) {
			if(ret == -2)
				return -5;
            return -2;
        }
        res_len = body_len;
    }
    // chunkµÄ
    else if (header.find("Transfer-Encoding: chunked") != std::string::npos){
        bool is_chunk_len_seg = true;
        char* chunk_head = res_buf;
        int chunk_last_len = 0;
        int chunk_total_len = 0;
        int chunk_seg_start = 0;

        char* seg_end_pos = NULL;
        char** strtol_endptr = NULL;
        int header_len = 0;
        int recv_len = ret;
        int rd_ret = 0;
        int each_size = 1024;
        while (1) {
            while (1) {
                if (is_chunk_len_seg) {
                    seg_end_pos = strstr(chunk_head + chunk_seg_start, "\r\n");
                    if (seg_end_pos != NULL && seg_end_pos - chunk_head  + 2 < recv_len - header_len) {
                        chunk_last_len = strtol(chunk_head + chunk_seg_start, strtol_endptr, 16);
                        if (chunk_last_len == 0) {
                            *(chunk_head + chunk_total_len) = 0;
                            res_len = chunk_total_len;
                            return 0;
                        }
                        is_chunk_len_seg = false;
                        chunk_seg_start = (seg_end_pos - chunk_head) + 2;
                        // check last len
                    }
                    else {
                        break;
                    }
                }
                else {
                    // recv enough data
                    if (chunk_seg_start + header_len + chunk_last_len + 2 < recv_len) {
                        memmove(chunk_head + chunk_total_len, chunk_head + chunk_seg_start, chunk_last_len);
                        chunk_total_len += chunk_last_len;
                        is_chunk_len_seg = true;
                        chunk_seg_start += chunk_last_len;
                        chunk_seg_start += 2;
                    }
                    else {
                        break;
                    }
                }
            }
            timeval timeout = {
                0,
                recv_timeout * 1000
            };
            rd_ret = read_data(fd, res_buf + recv_len, each_size, &timeout);
            if (rd_ret > 0) {
                recv_len += rd_ret;
            }
            else {
                return -4;
            }
        }
    }
    else {
		res_len = 0;
        return 10;
    }
    return 0;
}
