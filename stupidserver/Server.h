#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <istream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define TIMEOUT 1000

class Server
{
public:

	WSADATA wsadata;
	SOCKET client_socket;
	SOCKET server_socket;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	std::string port;
	
	Server(std::string port) {};



	std::vector<char> readContents(const std::string& filename);
	std::string httpResponse(std::vector<char>& content);
	void setupServer(std::string port);






	




};
