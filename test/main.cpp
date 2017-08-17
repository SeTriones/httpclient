#include <stdlib.h>
#include <stdio.h>
#include <HttpClient.hpp>
#include <service_log.hpp>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
	_INFO("test starts");
	HttpClient* client = new HttpClient(1024*1024*30, 30);
	FILE* f = fopen("face_image.jpg", "rb");
	if (f == NULL) {
		_ERROR("open face_image fail");
		return -1;
	}
	struct stat statbuf;
	stat("face_image.jpg", &statbuf);
	char* file_content = (char*)malloc(statbuf.st_size);
	int read_cnt = fread(file_content, 1, statbuf.st_size, f);
	if (read_cnt != (int)statbuf.st_size) {
		_ERROR("read fail,file size=%ld,read_cnt=%d", statbuf.st_size, read_cnt);
		return -1;
	}
	int ret = client->Connect("127.0.0.1:11086", 30);
	if (ret != 0) {
		_ERROR("connect fail,ret=%d", ret);
		return -1;
	}
	Form form;
	form.insert(std::pair<std::string, std::string>("a", "test msg a"));
	form.insert(std::pair<std::string, std::string>("b", "test msg b"));
	std::vector<FormFile> files;
	std::string reply_header;
	std::string reply_body;

	FormFile file1;
	file1.key = std::string("file1");
	file1.file_name = std::string("face_image.jpg");
	file1.file_content = file_content;
	file1.file_len = read_cnt;
	files.push_back(file1);
	
	ret = client->PostForm("/hello", form, files, reply_header, reply_body);
	if (ret != 0) {
		_ERROR("post form fail,ret=%d", ret);
		return -1;
	}
	_INFO("reply=%s", reply_body.c_str());
	delete client;
	_INFO("test done");
	return 0;
}
