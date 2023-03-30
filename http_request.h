#include "http_parser.h"
#include <bits/stdc++.h>
using namespace std;

struct http_request {
	enum request_status {
		REQUEST_INVALID,
		REQUEST_VALID
	};

	http_request(const string &s);
	std::map<std::string, std::string> get_headers();
	std::string get_url();
	std::string get_method();
	std::string get_body();
	std::string get_path();
	std::string get_query_string();
	bool is_valid();
	bool is_multipart();

	const vector<std::string>& get_multipart_data();

private:
	char *m_data = nullptr;
	int m_len = 0;
	bool m_request_status = REQUEST_INVALID;

	using self_type = http_request;

	http_parser m_parse;
	http_parser_settings m_settings;
	std::string m_latest_field;
	std::string m_url;
	std::string m_method;
	std::string m_path;
	std::string m_query_string;
	std::map<std::string, std::string> m_headers;
	std::string m_body_buf;
	bool m_is_multipart;
	std::string m_multipart_boundary;

	vector<string> m_multipart_data;

	void set_header(const std::string &value);
	void set_latest_field(std::string);
	void set_url(std::string);
	void set_method(std::string);
	void append_body(std::string);
	void set_request_status(request_status);
	static int on_header_field_cb(http_parser *parser, const char *buf, size_t len);
	static int on_url_cb(http_parser *parser, const char *buf, size_t len);
	static int on_header_value_cb(http_parser *parser, const char *buf, size_t len);
	static int on_headers_complete_cb(http_parser *parser);
	static int on_body_cb(http_parser *parser, const char *buf, size_t len);
	static int on_message_complete_cb(http_parser *parser);
};
