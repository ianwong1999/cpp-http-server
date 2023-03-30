#include "http_request.h"
#include <bits/stdc++.h>
using namespace std;

http_request::http_request(const string &s) {
	http_parser_init(&m_parse, HTTP_REQUEST);
	m_settings = {
		.on_message_begin = 0
			,.on_url = on_url_cb
			,.on_status = 0
			,.on_header_field = on_header_field_cb
			,.on_header_value = on_header_value_cb
			,.on_headers_complete = on_headers_complete_cb
			,.on_body = on_body_cb
			,.on_message_complete = on_message_complete_cb
			,.on_chunk_header = 0
			,.on_chunk_complete = 0
	};

	m_is_multipart = false;

	m_parse.data = this;

	m_data = const_cast<char*>(s.c_str());
	m_len = s.size();
	http_parser_execute(&m_parse, &m_settings, m_data, m_len);
}


void http_request::set_latest_field(std::string latest_field) {
	m_latest_field = latest_field;
}

void http_request::set_header(const std::string &value) {
	m_headers[m_latest_field] = value;

	if (m_latest_field == "Content-Type" && value.find("multipart/form-data") != string::npos) {
		m_is_multipart = true;		 
		auto boundary_pos = value.find("boundary=");
		boundary_pos += strlen("boundary=");
		m_multipart_boundary = value.substr(boundary_pos);
	}
}

void http_request::set_url(std::string url) {
	m_url = url;

	if (auto pos = url.find("?"); pos != string::npos) {
		m_path = url.substr(0, pos);
		m_query_string = url.substr(pos + 1);
	} else {
		m_path = url;
	}
}

void http_request::set_method(std::string method) {
	m_method = method;
}

void http_request::append_body(std::string body_part) {
	m_body_buf += body_part;
}

void http_request::set_request_status(request_status status) {
	m_request_status = status;

	if (m_is_multipart) {
		size_t pos = 0;
		while (pos < m_body_buf.size()) {
			auto nxt = m_body_buf.find("--" + m_multipart_boundary, pos);

			if (nxt == string::npos) {
				break;
			}

			if (nxt != pos && m_multipart_data.size() > 0) {
				m_multipart_data.back() += m_body_buf.substr(pos, nxt - pos);
			}

			m_multipart_data.push_back("");
			pos = nxt + 2 + m_multipart_boundary.size();
		}

		m_multipart_data.pop_back();
	}
}

std::map<std::string, std::string> http_request::get_headers() {
	return m_headers;
}

std::string http_request::get_url() {
	return m_url;
}

std::string http_request::get_method() {
	return m_method;
}

std::string http_request::get_body() {
	return m_body_buf;
}

std::string http_request::get_path() {
	return m_path;
}

std::string http_request::get_query_string() {
	return m_query_string;
}

const vector<std::string>& http_request::get_multipart_data() {
	return m_multipart_data;
}

bool http_request::is_valid() {
	return m_request_status == REQUEST_VALID;
}

bool http_request::is_multipart() {
	return m_is_multipart;
}

int http_request::on_header_field_cb(http_parser *parser, const char *buf, size_t len) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	impl->set_latest_field(std::string(buf, len));
	return 0;
}

int http_request::on_header_value_cb(http_parser *parser, const char *buf, size_t len) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	impl->set_header(std::string(buf, len));
	return 0;
}

int http_request::on_url_cb(http_parser *parser, const char *buf, size_t len) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	impl->set_url(std::string(buf, len));
	return 0;
}

int http_request::on_headers_complete_cb(http_parser *parser) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	std::string method = http_method_str((http_method)parser->method);
	impl->set_method(method);
	return 0;
}

int http_request::on_body_cb(http_parser *parser, const char *buf, size_t len) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	impl->append_body(std::string(buf, len));
	return 0;
}

int http_request::on_message_complete_cb(http_parser *parser) {
	self_type *impl = reinterpret_cast<self_type*>(parser->data);
	impl->set_request_status(REQUEST_VALID);
	return 0;
}
