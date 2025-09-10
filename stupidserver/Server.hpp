#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Server
{
private:
	WSADATA wsadata;
	SOCKET server_socket = INVALID_SOCKET;
	SOCKET client_socket = INVALID_SOCKET;
	int iResult;
	int bytesReceived;
	char recvBuf[DEFAULT_BUFLEN];
	int recvBufLen = DEFAULT_BUFLEN;
	struct addrinfo* result=NULL;
	struct addrinfo hints;
	struct addrinfo* p;

public:
	Server();
	SOCKET acceptConnetion(SOCKET serverSocket, SOCKET clientSocket);
	SOCKET receive(SOCKET clientSocket);
	void resultMessage(std::string error, int result);
	std::vector<char> readContents(std::string filename);


};