#include <bits/stdc++.h>
#include "http_request.h"
#include "http_response.h"
using namespace std;

class http_server {
public:
	void run(int port);	
	void add_get(const string &path, function<http_response(http_request)> callback) {
		add_route("GET", path, callback);
	}
	void add_post(const string &path, function<http_response(http_request)> callback) {
		add_route("POST", path, callback);
	}
	void add_put(const string &path, function<http_response(http_request)> callback) {
		add_route("PUT", path, callback);
	}
	void add_delete(const string &path, function<http_response(http_request)> callback) {
		add_route("DELETE", path, callback);
	}

	void add_route(const string &method, const string &path, function<http_response(http_request)> callback) {
		routes[make_pair(method, path)] = callback;
	}
private:
	string process_request(const string &request);
	map<pair<string, string>, function<http_response(http_request)>> routes;

	int port;
};
