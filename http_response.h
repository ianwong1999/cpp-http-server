#include <bits/stdc++.h>
using namespace std;

struct http_response {
	int status;
	string message;

	http_response(int status, const string& message, string file_type = "") {
		if (status >= 400) {
			std::string header_buff, body_buff;
			char output[4 * 1024];
			// body
			body_buff += "<html><title>Error</title>";
			body_buff += "<body bgcolor=\"ffffff\">";
			body_buff += std::to_string(status) + " : " + message + "\n";
			body_buff += "<p>" + message + "</p>";
			body_buff += "<hr><em>WebServer</em></body></html>";

			// header
			header_buff += "HTTP/1.1 " + std::to_string(status) + " " + message + "\r\n";
			header_buff += "Server: WebServer\r\n";
			header_buff += "Content-type: text/html\r\n";
			header_buff += "Connection: close\r\n";
			header_buff += "Content-length: " + std::to_string(body_buff.size()) + "\r\n\r\n";
			this->status = status;
			this->message = header_buff + body_buff;
		} else {
			std::string header_buff;
			header_buff += "HTTP/1.1 " + std::to_string(status) + " OK\r\n";
			header_buff += "Content-type: " + file_type + "\r\n";
			header_buff += "Content-length: " + std::to_string(message.size()) + "\r\n";
			header_buff += "Server: WebServer\r\n";
			header_buff += "\r\n";
			this->status = status;
			this->message = header_buff + message;
		}
	}
};
