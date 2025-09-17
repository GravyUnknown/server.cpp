#include "Server.hpp"

bool Server::resultMessage(const std::string& function, int result)
{
	if (result == SOCKET_ERROR || result == INVALID_SOCKET)
	{
		const std::string& errorMessage = function + "failed with: " + std::to_string(WSAGetLastError());
		throw std::runtime_error(errorMessage);
		return false;

	}
	else {
		std::cout << function << " succeeded" << std::endl;
		return true;
	}
}

Server::Server()
{
	try {
		initializeWinsock();
		setupServer();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

Server::~Server()
{
	WSACleanup();
}

SOCKET Server::acceptConnetion(SOCKET serverSocket, SOCKET clientSocket)
{

	clientSocket = accept(serverSocket, NULL, NULL);
	resultMessage("clientsocket", clientSocket);

	closesocket(m_ServerSocket);

	receive(clientSocket);

	return 0;

}

SOCKET Server::receive(SOCKET clientSocket)
{

	bytesReceived = recv(clientSocket, recvBuf, recvBufLen, 0);
	std::cout << bytesReceived << '\n';
	std::cout << recvBuf << std::endl;
	resultMessage("recv", bytesReceived);
	int iSendResult = send(clientSocket, recvBuf, recvBufLen, 0);
	resultMessage("send", iSendResult);
	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	
	return 0;
}


std::string& Server::readContents(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	std::stringstream buffer;

	buffer << file.rdbuf();
	m_Buffer = buffer.str();

	return m_Buffer;




}

const std::string_view Server::s_ParseRequest(const std::string& recvBuf)
{

	std::istringstream buf(recvBuf);
	std::string line;

	while (std::getline(buf, line)) 
	{
		if (line.find("/about") != std::string::npos)
		{
			return "/about";
		}
		else
		{
			return "/";
		}
	}


}

const bool Server::keepConnectionAlive(bool keepAlive)
{
	return keepAlive;
}


void Server::sendMessage(SOCKET clientSocket)
{


	do
	{
		bytesReceived = recv(clientSocket, recvBuf, sizeof(recvBuf), 0);
		resultMessage("recv", bytesReceived);
		std::cout << recvBuf << std::endl;
		std::cout << s_ParseRequest(recvBuf) << std::endl;
		const std::string& respondMessage =
			"HTTP/1.1 200 OK \r\n"
			"CONTENT-TYPE: text/html \r\n"
			"CONTENT-LENGTH: " + std::to_string(m_Buffer.size()) + "\r\n"
			"Connection: keep-alive \r\n"
			"Keep-Alive: timeout=100\r\n"
			"\r\n" + m_Buffer;





		int	iSendResult = send(clientSocket, respondMessage.c_str(), respondMessage.size(), 0);
		resultMessage("html file send", iSendResult);

		if (bytesReceived <= 0)
		{
			keepConnectionAlive(false);
			std::cout << "Connection close or other error\n";
		}
	} while (keepConnectionAlive());





}

void Server::initializeWinsock()
{

	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		throw std::runtime_error("WSAStartup failed " + std::to_string(WSAGetLastError()));
	}
}

void Server::setupServer()
{

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		throw std::runtime_error("getaddrinfo failed with " + std::to_string(WSAGetLastError()));
		return;
	}

	m_ServerSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ServerSocket == INVALID_SOCKET)
	{
		throw std::runtime_error("creating socket failed with " + std::to_string(WSAGetLastError()));

		return;

	}

	iResult = bind(m_ServerSocket, result->ai_addr, (int)result->ai_addrlen);
	resultMessage("bind", iResult);

	freeaddrinfo(result);
	result = nullptr;

	iResult = listen(m_ServerSocket, SOMAXCONN);
	resultMessage("listen", iResult);

	m_ClientSocket = accept(m_ServerSocket, NULL, NULL);
	resultMessage("accept", m_ClientSocket);
	readContents("index.html");
	std::cout << "Buffer: " << m_Buffer << std::endl;
	sendMessage(m_ClientSocket);

	shutdown(m_ClientSocket, SD_SEND);
	closesocket(m_ClientSocket);

}
