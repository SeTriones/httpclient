#ifndef _HTTP_CLIENT_HPP_
#define _HTTP_CLIENT_HPP_
#include <string.h>
#include <string>
#include <base_nio_handler.hpp>
#include <pthread.h>
#include <service_log.hpp>
#include <HttpUtil.hpp>

class HttpClient : public base_nio_handler {
public:
	HttpClient(int _max_buf_len = 0, int _timeout = 30) {
		pthread_mutex_init(&mutex, NULL);
		if (_max_buf_len <= 0) {
			max_buf_len = 4096;
		}
		if (_timeout <= 0) {
			timeout = 30;
		}
		max_buf_len = _max_buf_len;
		timeout = _timeout;
		recv_timeout = timeout;
		send_timeout = timeout;
		_INFO("recv_timeout=%d,send_timeout=%d", recv_timeout, send_timeout);
		buf = (char*)malloc(max_buf_len);
		fd = -1;
	}

	~HttpClient() {
		pthread_mutex_destroy(&mutex);
		if (buf != NULL) {
			free(buf);
		}
	}

	int Get(const char* uri, std::string& header, std::string& body);
	int Post(const char* uri, const char* body, int body_len, std::string& header, std::string &reply_body);
	int PostForm(const char* url, Form& form, std::vector<FormFile> files, std::string& header, std::string& reply_body); 
	int Connect(std::string host, int timeout);
	int Close();
	void lock();
	void unlock();

private:
	char* buf;
	int max_buf_len;
	int timeout;
	std::string host;
	int fd;
	pthread_mutex_t mutex;

protected:
	int Recv(std::string& header, std::string& body);
};

#endif
