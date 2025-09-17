#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <array>
#include <vector>
#include <istream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
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
	SOCKET m_ServerSocket = INVALID_SOCKET;
	SOCKET m_ClientSocket = INVALID_SOCKET;
	std::string m_Buffer;
	std::string m_RecvBuf;
	int iResult;
	int bytesReceived;
	char recvBuf[DEFAULT_BUFLEN];
	int recvBufLen = DEFAULT_BUFLEN;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	struct addrinfo* p;


public:
	Server();
	~Server();
	static const bool keepConnectionAlive(bool keepalive=false);
	SOCKET acceptConnetion(SOCKET serverSocket, SOCKET clientSocket);
	SOCKET receive(SOCKET clientSocket);
	bool resultMessage(const std::string& error, int result);
	std::string& readContents(const std::string& filename);
	static const std::string_view s_ParseRequest(const std::string& recvBuf);
	void sendMessage(SOCKET clientSocket) ;
	void initializeWinsock();
	void setupServer();


};