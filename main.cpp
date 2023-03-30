#include "http_server.h"
#include <bits/stdc++.h>
using namespace std;

int main() {
	http_server server;
	server.add_get("/time", [] (http_request request) {
		auto cur_time = chrono::system_clock::to_time_t(chrono::system_clock::now());

		return http_response(200, "Current time: " + string(ctime(&cur_time)));
	});
	server.run(8080);
	return 0;
}

