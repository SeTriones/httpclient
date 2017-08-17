#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP

#include <vector>
#include <tr1/unordered_map>
#include <string>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
	UNKNOWN,
	TEXT,
	FORM,
	MULTIPART_FORM,
	JSON,

	HTTP_CONTENT_TYPY_CNT
} HttpContentType;

const static char* HttpContentTypes[HTTP_CONTENT_TYPY_CNT] = {
	"",
	"text/plain",
	"application/x-www-form-urlencoded",
	"multipart/form-data",
	"application/json",
};

typedef std::tr1::unordered_map<std::string, std::string> Form;
typedef struct {
	std::string key;
	std::string file_name;
	const char* file_content;
	int file_len;
} FormFile;

static int makeUpForm(Form& form, std::vector<FormFile> files, int& formType, std::string& msg, std::string& boundary) {
	if (files.size() == 0) {
		formType = FORM;
		Form::iterator iter;
		std::string formString = "";
		for (iter = form.begin(); iter != form.end(); iter++) {
			formString = formString + iter->first + "=" + iter->second + "&";
		}
		int len = formString.length();
		msg = formString.substr(0, len -1);
		return 0;
	} else {
		formType = MULTIPART_FORM;
		boundary = "WebKitFormBoundary7MA4YWxkTrZu0gW";
		std::string formString = "";
		const std::string sep = "--" + boundary;
		const std::string end = "--" + boundary + "--";
		Form::iterator iter;
		for (iter = form.begin(); iter != form.end(); iter++) {
			formString = formString + sep + "\r\n" + "Content-Disposition: form-data; name=\"" + iter->first+ "\"\r\n\r\n" + iter->second + "\r\n";
		}
		int file_cnt = files.size();
		for (int i = 0; i < file_cnt; i++) {
			std::string seg_header = sep + "\r\n" + "Content-Disposition: form-data; name=\"" + files[i].key + "\"; filename=\"" + files[i].file_name + "\"\r\n";
			seg_header = seg_header + "Content-Type: application/octet-stream\r\n\r\n";
			seg_header = seg_header + std::string(files[i].file_content, files[i].file_len);
			seg_header += "\r\n";
			formString += seg_header;
		}
		formString += end;
		msg = formString;
		return 0;
	}
	return 0;
}

#endif
