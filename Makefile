all:
	g++ -std=c++2a -O3 main.cpp http_server.cpp http_request.cpp http_parser.c -o main
