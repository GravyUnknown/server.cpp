#include "Server.hpp"
#include <fstream>
#include <iterator>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

bool Server::ResultMessage(const std::string &function, int result) {
  if (result == SOCKET_ERROR || result == INVALID_SOCKET) {
    const std::string errorMessage =
        function + "failed with: " + std::to_string(WSAGetLastError());
    throw std::runtime_error(errorMessage);
    return false;

  } else {
    std::cout << function << " succeeded" << std::endl;
    return true;
  }
}

Server::~Server() { WSACleanup(); }

std::string Server::ReadContents(const std::filesystem::path &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!(file.is_open()))
    std::cerr << "File not found" << std::endl;
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

Server::ReturnStatus Server::ParseRequest(const std::string &recvBuf) {
  std::istringstream buf(recvBuf);
  std::string method, route, version;
  static std::map<std::string, std::string> routes{ {"/", this->indexHTML},
                                                   {"/about", this->aboutHTML} };

  if (!(buf >> method >> route >> version))
    return {Status::ERR_BADREQUEST, " "};

  if (method != "GET")
    return {Status::ERR_INVALIDMETHOD, " "};

  return routes.contains(route)
             ? Server::ReturnStatus{Status::OK, routes[route]}
             : Server::ReturnStatus{Status::ERR_NOTFOUND, " "};
}

int Server::SendMessage() {

  std::string fullRequest;
  fullRequest.reserve(1024);
  Server::ReturnStatus parsingResult;
  while (true) {
    mBytesReceived = recv(mClientSocket, mRecvBuf, sizeof(mRecvBuf) - 1, 0);

    mRecvBuf[mBytesReceived] = '\0';
    fullRequest.append(mRecvBuf);

    if (fullRequest.find("\r\n\r\n") != std::string::npos) {
      break;
    }
  }
  std::cout << fullRequest << std::endl;

  
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
  parsingResult = ParseRequest(fullRequest);

  switch (parsingResult.returnCode) {
  case Status::ERR_BADREQUEST:
    mBuffer = errInvalid;
    break;
  case Status::ERR_INVALIDMETHOD:
    mBuffer = errMethod;
    break;
  case Status::ERR_NOTFOUND:
    mBuffer = err404;
    break;
  case Status::OK:
    std::string body = parsingResult.route;
    // m_Buffer = successHeader + std::to_string(body.size()) + "\r\n\r\n" +
    // body;
    mBuffer = successHeader + std::to_string(body.size()) + "\r\n\r\n" + body;
    break;
  }

  send(mClientSocket, mBuffer.c_str(), static_cast<int>(mBuffer.size()), 0);
  return 1;
}

void Server::InitializeWinsock() {

  mIResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (mIResult != 0) {
    std::cerr << "Startup failed: " << WSAGetLastError() << std::endl;
  }
}

void Server::SetupServer() {
   static const std::filesystem::path currentPath = std::filesystem::relative("stupidserver");
   indexHTML = ReadContents(currentPath / "index.html");
   aboutHTML = ReadContents(currentPath / "about.html");
   
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  mIResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &serverResult);
  if (mIResult != 0) {
    std::cerr << "getaddrinfo failed: " << WSAGetLastError() << std::endl;
    return;
  }

  mServerSocket =
      socket(serverResult->ai_family, serverResult->ai_socktype, serverResult->ai_protocol);
  if (mServerSocket == INVALID_SOCKET) {
    throw std::runtime_error("creating socket failed with " +
                             std::to_string(WSAGetLastError()));
  }

  mIResult = bind(mServerSocket, serverResult->ai_addr, (int)serverResult->ai_addrlen);
  // resultMessage("binding ", m_iResult);

  freeaddrinfo(serverResult);
  serverResult = nullptr;

  mIResult = listen(mServerSocket, SOMAXCONN);
  // resultMessage("listening ", m_iResult);
  std::cout << "Listening on " << DEFAULT_PORT << std::endl;
  while (true) {

    mClientSocket = accept(mServerSocket, NULL, NULL);

    if (mClientSocket == INVALID_SOCKET) {
      continue;
    }

    SendMessage();
    shutdown(mClientSocket, SD_SEND);
    closesocket(mClientSocket);
  }
}
