#include "Server.hpp"

void Server::resultMessage(std::string function, int result)
{
	if (result == SOCKET_ERROR)
	{
		std::cout << function << "failed with " << WSAGetLastError() << std::endl;
		WSACleanup();
	}
	else { std::cout << function << " succeeded" << std::endl; }
}
Server::Server()
{
	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		std::cout << "Error with WSAStartup " << WSAGetLastError() << std::endl;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0 )
	{
		std::cout << "Error with getaddrinfo" << WSAGetLastError() << std::endl;
		WSACleanup();
		return;
	}

	server_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server_socket == INVALID_SOCKET)
	{
		std::cout << "Socket failed with " << WSAGetLastError() << std::endl;
		WSACleanup();
		return;

	}

	iResult = bind(server_socket, result->ai_addr, (int)result->ai_addrlen);
	resultMessage("bind", iResult);

	freeaddrinfo(result);

	iResult = listen(server_socket, SOMAXCONN);
	resultMessage("listen", iResult);
	acceptConnetion(server_socket, client_socket);

}

SOCKET Server::acceptConnetion(SOCKET serverSocket, SOCKET clientSocket)
{

	clientSocket = accept(serverSocket, NULL, NULL);
	resultMessage("clientsocket", clientSocket);

	closesocket(server_socket);

	receive(clientSocket);

	return 0;

}

SOCKET Server::receive(SOCKET clientSocket)
{
	do
	{
		bytesReceived = recv(clientSocket, recvBuf, recvBufLen, 0);
		resultMessage("recv", iResult);

		std::cout << recvBuf << std::endl;



		if (bytesReceived > 0)
		{
			std::cout << "Bytes received " << bytesReceived << std::endl;

			int iSendResult = send(clientSocket, recvBuf, bytesReceived, 0);
			resultMessage("send ", iSendResult);
		}

		else if (bytesReceived <= 0)
		{
			std::cout << "Connection closed or error " << WSAGetLastError() << std::endl;
		}
	} while (bytesReceived > 0);

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}


std::vector<char> Server::readContents(std::string filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Cannot open file " << filename << std::endl;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);

	if (!file.read(buffer.data(), size))
	{
		std::cerr << "Cannot read " << filename << std::endl;
	}

	return buffer;



}

std::string Server::parseRequest(std::string recvBuf)
{

	std::istringstream buf(recvBuf);
	std::string line;

	while (std::getline(buf, line)) {

		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		if (line.find("connection: keep-alive") != std::string::npos)
		{
			keepAlive = true;
			
			break;
		}

		if (line.find("get: /") != std::string::npos)
		{
			return "/";
		}
		else if (line.find("get: /about") != std::string::npos)
		{
			return "/about";
		}


		return "";


	}
