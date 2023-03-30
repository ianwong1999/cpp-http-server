#include "http_server.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
using namespace std;

#define MAX_CONN        10000
#define MAX_EVENTS      32
#define BUF_SIZE        30000
#define MAX_LINE        256

static void epoll_ctl_add(int epfd, int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl()\n");
		exit(1);
	}
}

static void set_sockaddr(struct sockaddr_in *addr, int port) {
	bzero((char *)addr, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_port = htons(port);
}

static int setnonblocking(int sockfd) {
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

void http_server::run(int port) {
	int i;
	int n;
	int epfd;
	int nfds;
	int listen_sock;
	int conn_sock;
	int socklen;
	char buf[BUF_SIZE];
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	struct epoll_event events[MAX_EVENTS];

	this->port = port;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	set_sockaddr(&srv_addr, port);
	bind(listen_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

	setnonblocking(listen_sock);
	listen(listen_sock, MAX_CONN);

	epfd = epoll_create(1);
	epoll_ctl_add(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

	socklen = sizeof(cli_addr);
	for (;;) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == listen_sock) {
				/* handle new connection */
				conn_sock =
					accept(listen_sock,
							(struct sockaddr *)&cli_addr,
							(socklen_t*)&socklen);

				inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr),
						buf, sizeof(cli_addr));
				printf("[+] connected with %s:%d\n", buf,
						ntohs(cli_addr.sin_port));

				setnonblocking(conn_sock);
				epoll_ctl_add(epfd, conn_sock,
						EPOLLIN | EPOLLET | EPOLLRDHUP |
						EPOLLHUP);
			} else if (events[i].events & EPOLLIN) {
				/* handle EPOLLIN event */
				string data = "";
				for (;;) {
					bzero(buf, sizeof(buf));
					n = read(events[i].data.fd, buf,
							sizeof(buf));
					if (n <= 0 /* || errno == EAGAIN */ ) {
						break;
					} else {
						data.append(buf);
						printf("[+] data: %s\n", buf);
					}
				}
				string res = process_request(data);
				write(events[i].data.fd, res.c_str(), res.size());
			} else {
				printf("[+] unexpected\n");
			}
			/* check if the connection is closing */
			if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
				printf("[+] connection closed\n");
				epoll_ctl(epfd, EPOLL_CTL_DEL,
						events[i].data.fd, NULL);
				close(events[i].data.fd);

				continue;
			}
		}
	}
}

const std::unordered_map<std::string, std::string> mime_type = {
	{".html",  "text/html"},
	{".css",   "text/css"},
	{".xml",   "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt",   "text/plain"},
	{".png",   "image/png"},
	{".gif",   "image/gif"},
	{".jpg",   "image/jpeg"},
	{".jpeg",  "image/jpeg"},
	{".txt",   "text/plain"},
	{".rtf",   "application/rtf"},
	{".pdf",   "application/pdf"},
	{".word",  "application/nsword"},
	{".au",    "audio/basic"},
	{".mpeg",  "video/mpeg"},
	{".mpg",   "video/mpeg"},
	{".avi",   "video/x-msvideo"},
	{".gz",    "application/x-gzip"},
	{".tar",   "application/x-tar"},
};

string http_server::process_request(const string &request_raw_string) {
	http_request request(request_raw_string);

	const auto& method = request.get_method();
	const auto& path = request.get_path();

	auto ite = this->routes.find({method, path});

	if (ite != this->routes.end()) {
		return ite->second(request).message;
	}

	if (request.get_method() == "GET") {
		string file_name = "";


		if (request.get_path() == "/") {
			file_name = "index.html";
		} else {
			file_name = request.get_path().substr(1);
		}

		struct stat sbuf;
		if (stat(file_name.c_str(), &sbuf) < 0) {
			return http_response(404, "NOT FOUND!").message;
		}

		auto dot_pos = file_name.find_last_of(".");
		string file_type = "";
		if (dot_pos == std::string::npos) {
			file_type = "text/html";
		} else {
			std::string suffix = file_name.substr(dot_pos);
			auto it = mime_type.find(suffix);
			if (it == mime_type.cend()) {
				file_type = "text/html";
			} else {
				file_type = it->second;
			}
		}

		int src_fd = open(file_name.c_str(), O_RDONLY, 0);

		if (src_fd < 0) {
			return http_response(404, "NOT FOUND!").message;
		}

		void *mmapRet = mmap(nullptr, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);

		close(src_fd);

		if (mmapRet == (void *) (-1)) {
			munmap(mmapRet, sbuf.st_size);
			return http_response(404, "NOT FOUND!").message;
		}

		char *src_addr = static_cast<char *>(mmapRet);
		string body = string(src_addr, src_addr + sbuf.st_size);
		munmap(mmapRet, sbuf.st_size);

		return http_response(200, body, file_type).message;
	}

	return http_response(404, "NOT FOUND!").message;
}
