#undef UNICODE

#define WIN32_LEAN_AND_MEAN

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


int main()
{
	WSADATA wsadata;
	int iResult;

	SOCKET server = INVALID_SOCKET;
	SOCKET client = INVALID_SOCKET;

	char buf[DEFAULT_BUFLEN];
	int buflen = DEFAULT_BUFLEN;
	std::ifstream stream("index.html");

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	if (WSAStartup((2, 2), &wsadata) != 0)
	{
		std::cout << "failed to launch wsa " << WSAGetLastError();
		WSACleanup();
		return 1;
	}


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0)
	{
		std::cout << "getaddrinfo failed with " << WSAGetLastError();
		WSACleanup();
		return 1;
	}

	server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server == INVALID_SOCKET)
	{

		std::cout << "failed to launch socket" << WSAGetLastError();
		WSACleanup();
		return 1;
	}

	if (bind(server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) 
	{
		std::cout << "Binding failed " << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);


	if (listen(server, SOMAXCONN) == INVALID_SOCKET)
	{
		std::cout << "Listening failed " << WSAGetLastError();
		closesocket(server);
		WSACleanup();
		return 1;
	}
	
	client = accept(server, NULL, NULL);
	if (client == SOCKET_ERROR)
	{
		std::cout << "Client failed to accept " << WSAGetLastError();
		closesocket(server);
		WSACleanup();
		return 1;
	}


	do
	{
		iResult = recv(client, buf, buflen, 0);
		stream.seekg(0, std::ios::end);
		int length = stream.tellg();
		stream.seekg(0, std::ios::beg);

		stream.read(buf, length);

		if (send(client, buf, length, 0) == SOCKET_ERROR)
		{
			std::cout << "Send failed " << WSAGetLastError();
			closesocket(server);
			WSACleanup();
			return 1;
		}


	} while (iResult > 0);


	if (shutdown(client, SD_SEND) == SOCKET_ERROR)
	{
		std::cout << "Shutdown failed " << WSAGetLastError();
		closesocket(client);
		WSACleanup();
		return 1;
	}


	closesocket(client);
	WSACleanup();


}