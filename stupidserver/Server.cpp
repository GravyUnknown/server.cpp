#include "Server.hpp"
#include <string>
#include <ws2tcpip.h>

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
  std::ifstream file(filename, std::ios::binary);
  if (!(file.is_open()))
    std::cerr << "File not found" << std::endl;
  std::stringstream buffer;
  buffer << file.rdbuf();
  m_Buffer = buffer.str();
  return m_Buffer;
}

Server::ReturnStatus Server::s_ParseRequest(const std::string &recvBuf) {
  std::istringstream buf(recvBuf);
  std::string method, route, version;
  static std::map<std::string, std::string> routes{{"/", "about.html"},
                                                   {"/about", "about.html"}};

  if (!(buf >> method >> route >> version))
    return {Status::ERR_BADREQUEST, ""};

  if (method != "GET")
    return {Status::ERR_INVALIDMETHOD, ""};

  return routes.contains(route)
             ? Server::ReturnStatus{Status::OK, routes[route]}
             : Server::ReturnStatus{Status::ERR_NOTFOUND, " "};
}

int Server::sendMessage(SOCKET clientSocket) {

  recv(m_ClientSocket, m_RecvBuf, sizeof(m_RecvBuf), 0);
  std::cout << m_RecvBuf << std::endl;
  std::string body;
  static const std::string &successHeader = "HTTP/1.1 200 OK\r\n"
                                            "CONTENT-TYPE: text/html \r\n"
                                            "Connection: close \r\n"
                                            "CONTENT-LENGTH: ";
  static const std::string &errInvalid =
      "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: "
      "Invalid format";
  static const std::string &errMethod =
      "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: "
      "text/plain\r\n\r\nError: Use GET";
  static const std::string &err404 = "HTTP/1.1 404 Not Found\r\n"
                                     "CONTENT-TYPE:text/plain "
                                     "\r\n\r\n"
                                     "Error: Path not found";
  Server::ReturnStatus parsingResult = s_ParseRequest(m_RecvBuf);

  switch (parsingResult.returnCode) {
  case Status::ERR_BADREQUEST:
    m_Buffer = errInvalid;
    break;
  case Status::ERR_INVALIDMETHOD:
    m_Buffer = errMethod;
    break;
  case Status::ERR_NOTFOUND:
    m_Buffer = err404;
    break;
  case Status::OK:
    body = readContents(parsingResult.route);
    m_Buffer = successHeader + std::to_string(body.size()) + "\r\n\r\n" + body;
    break;
  }

  send(clientSocket, m_Buffer.c_str(), m_Buffer.size(), 0);
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

    freeaddrinfo(result);
  }

  m_iResult = bind(m_ServerSocket, result->ai_addr, (int)result->ai_addrlen);
  // resultMessage("binding ", m_iResult);

  freeaddrinfo(result);
  result = nullptr;

  m_iResult = listen(m_ServerSocket, SOMAXCONN);
  // resultMessage("listening ", m_iResult);
  std::cout << "Listening on " << DEFAULT_PORT << std::endl;
  while (true) {

    m_ClientSocket = accept(m_ServerSocket, NULL, NULL);

    if (m_ClientSocket == INVALID_SOCKET) {
      continue;
    }

    sendMessage(m_ClientSocket);

    shutdown(m_ClientSocket, SD_SEND);
    closesocket(m_ClientSocket);
  }
}
