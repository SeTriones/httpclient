#include <HttpClient.hpp>
#include <service_log.hpp>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>

int HttpClient::Get(const char* url, std::string& header, std::string& body) {
	header = "";
	body = "";
	int ret = -1;
	const int max = max_buf_len;
	int len = 0;
	len += snprintf(buf, max, "GET %s HTTP/1.1\r\n", url);
	len += snprintf(buf + len, max - len, "Host: %s\r\n", host.c_str());
	len += snprintf(buf + len, max - len, "Content-Length: %d\r\n", 0);
	len += snprintf(buf + len, max - len, "Connection: close\r\n\r\n");
	int write_cnt = writen_timeout(fd, buf, len, send_timeout);
	if (write_cnt == len) {
		ret = 0;
	}
	ret = Recv(header, body);
	return ret;
}

int HttpClient::Post(const char* url, const char* body, int body_len, std::string& header, std::string& reply_body) {
	header = "";
	reply_body = "";
	int ret = -1;
	const int max = max_buf_len;
	int len = 0;
	len += snprintf(buf, max, "POST %s HTTP/1.1\r\n", url);
	len += snprintf(buf + len, max - len, "Host: %s\r\n", host.c_str());
	len += snprintf(buf + len, max - len, "Content-Length: %d\r\n", body_len);
	len += snprintf(buf + len, max - len, "Connection: close\r\n\r\n");
	if (body_len + len + 1 <= max) {
		memcpy(buf + len, body, body_len);
		len += body_len;
		buf[len] = 0;
	}
	int write_cnt = writen_timeout(fd, buf, len, send_timeout);
	if (write_cnt == len) {
		ret = 0;
	}
	ret = Recv(header, reply_body);
	return ret;
}

int HttpClient::PostForm(const char* url, Form& form, std::vector<FormFile> files, std::string& header, std::string& reply_body) {
	std::string body;
	int formType;
	std::string boundary;

	makeUpForm(form, files, formType, body, boundary);
	std::string content_type = "";
	if (formType != UNKNOWN) {
		content_type = content_type + HttpContentTypes[formType];
		if (boundary != "") {
			content_type = content_type + "; boundary=" + boundary;
		}
	}
	int body_len = body.length();

	header = "";
	reply_body = "";
	int ret = -1;
	const int max = max_buf_len;
	int len = 0;
	len += snprintf(buf, max, "POST %s HTTP/1.1\r\n", url);
	len += snprintf(buf + len, max - len, "Host: %s\r\n", host.c_str());
	if (content_type != "") {
		len += snprintf(buf + len, max - len, "Content-Type: %s\r\n", content_type.c_str());
	}
	len += snprintf(buf + len, max - len, "Content-Length: %d\r\n", body_len);
	len += snprintf(buf + len, max - len, "Connection: close\r\n\r\n");
	if (body_len + len + 1 <= max) {
		memcpy(buf + len, body.c_str(), body_len);
		len += body_len;
		buf[len] = 0;
	}

	int write_cnt = writen_timeout(fd, buf, len, send_timeout);
	if (write_cnt == len) {
		ret = 0;
	}
	ret = Recv(header, reply_body);
	return ret;
}

int HttpClient::Recv(std::string& header, std::string& body) {
	int rd_ret = read(fd, buf, 1);
	if (!((rd_ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) || 
				rd_ret == 1)) {
		_ERROR("disconnected by remote");
		return -10;
	}
	if (rd_ret < 0) {
		rd_ret = 0;
	}
	int ret = -1;
	int len = max_buf_len - rd_ret;
	ret = i_recv_http_res(fd, buf + rd_ret, len, header, recv_timeout);
	if (ret < 0) {
		return ret;
	}
	if (rd_ret == 1) {
		header = buf[0] + header;
	}
	body = std::string(buf + rd_ret, len);
	return ret;
}

int HttpClient::Connect(std::string _host, int connect_timeout) {
	host = _host;
	static addrinfo hints = { 0, AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, NULL, NULL, NULL };
	char ip[1024] = {0};
	char *pos;
	addrinfo *res;
	sockaddr_in* host_addr = NULL;
	const char* value = host.c_str();

	if ((pos = strchr((char*)value, ':')) != NULL) {
		strncpy(ip, value, pos - value);
		if (getaddrinfo(ip, pos + 1, &hints, &res)) {
			return -1;
		}
		else {
			host_addr = (sockaddr_in *)res->ai_addr;
		}
	} else {
		strncpy(ip, value, strlen(value));
		if (getaddrinfo(ip, "http", &hints, &res)) {
			return -1;
		}
		else {
			host_addr = (sockaddr_in *)res->ai_addr;
		}
	}

	fd = connectSocket(host_addr, connect_timeout);
	if (fd < 0) {
		return -2;
	}
	return 0;
}

int HttpClient::Close() {
	int ret = close(fd);
	if (ret < 0) {
		_ERROR("close %s err, errno=%d", host.c_str(), errno);
	}
	host = "NULL";
	return ret;
}

void HttpClient::lock() {
	pthread_mutex_lock(&mutex);
}

void HttpClient::unlock() {
	pthread_mutex_unlock(&mutex);
}
