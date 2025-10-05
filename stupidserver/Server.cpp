#include "Server.hpp"

bool Server::resultMessage(const std::string &function, int result) {
  if (result == SOCKET_ERROR || result == INVALID_SOCKET) {
    const std::string &errorMessage =
        function + "failed with: " + std::to_string(WSAGetLastError());
    throw std::runtime_error(errorMessage);
    return false;

  } else {
    std::cout << function << " succeeded" << std::endl;
    return true;
  }
}

Server::~Server() { WSACleanup(); }

std::string Server::readContents(const std::string &filename) {
  std::ifstream file(filename.c_str());
  std::stringstream buffer;
  buffer << file.rdbuf();
  m_Buffer = buffer.str();
  return m_Buffer;
}

std::string Server::s_ParseRequest(const std::string &recvBuf) {
  std::istringstream buf(recvBuf);
  std::string line;

  while (std::getline(buf, line)) {
    if (line.find("/about") != std::string::npos) {
      return "about.html";
    } else {
      return "index.html";
    }
  }

  return "";
}

int Server::sendMessage(SOCKET clientSocket) {

  recv(m_ClientSocket, m_RecvBuf, sizeof(m_RecvBuf), 0);
  readContents(s_ParseRequest(m_RecvBuf));

  const std::string &respondMessage = "HTTP/1.1 200 OK \r\n"
                                      "CONTENT-TYPE: text/html \r\n"
                                      "CONTENT-LENGTH: " +
                                      std::to_string(m_Buffer.size()) +
                                      "\r\n"
                                      "\r\n" +
                                      m_Buffer;

  send(clientSocket, respondMessage.c_str(), respondMessage.size(), 0);
  return 1;
}

void Server::initializeWinsock() {

  m_iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (m_iResult != 0) {
    throw std::runtime_error("WSAStartup failed " +
                             std::to_string(WSAGetLastError()));
  }
}

void Server::setupServer() {

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  m_iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
  if (m_iResult != 0) {
    throw std::runtime_error("getaddrinfo failed with " +
                             std::to_string(WSAGetLastError()));
    return;
  }

  m_ServerSocket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (m_ServerSocket == INVALID_SOCKET) {
    throw std::runtime_error("creating socket failed with " +
                             std::to_string(WSAGetLastError()));

    return;
  }

  m_iResult = bind(m_ServerSocket, result->ai_addr, (int)result->ai_addrlen);
  // resultMessage("binding ", m_iResult);

  freeaddrinfo(result);
  result = nullptr;

  m_iResult = listen(m_ServerSocket, SOMAXCONN);
  // resultMessage("listening ", m_iResult);
  while (true) {

    m_ClientSocket = accept(m_ServerSocket, NULL, NULL);

    if (m_ClientSocket == INVALID_SOCKET) {
      continue;
    }

    sendMessage(m_ClientSocket);
  }

  shutdown(m_ClientSocket, SD_SEND);
  closesocket(m_ClientSocket);
}
