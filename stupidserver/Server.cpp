#include "Server.hpp"
#include <fstream>
#include <iterator>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

// Allocating memory for static members
std::string Server::aboutHTML{};
std::string Server::indexHTML{};

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

std::string Server::SendMessage(Server::ReturnStatus& presult) {

  
  
  static const std::string_view successHeader = "HTTP/1.1 200 OK\r\n"
                                            "CONTENT-TYPE: text/html \r\n"
                                            "Connection: close \r\n"
                                            "CONTENT-LENGTH: ";
  static const std::string_view errInvalid =
      "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: "
      "Invalid format";
  static const std::string_view errMethod =
      "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: "
      "text/plain\r\n\r\nError: Use GET";
  static const std::string_view err404 = "HTTP/1.1 404 Not Found\r\n"
                                     "CONTENT-TYPE:text/plain "
                                     "\r\n\r\n"
                                     "Error: Path not found";
  

  switch (presult.returnCode) {
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
    std::string body = presult.route;
    // m_Buffer = successHeader + std::to_string(body.size()) + "\r\n\r\n" +
    // body;
    mBuffer = std::string(successHeader) + std::to_string(body.size()) + "\r\n\r\n" + body;
    break;
  }

  return mBuffer;
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

  std::fill(std::begin(events), std::end(events), WSAPOLLFD{ 0 });


  mIResult = getaddrinfo(NULL, DEFAULT_PORT.data(), &hints, &serverResult);
  if (mIResult != 0) {
    std::cerr << "getaddrinfo failed: " << WSAGetLastError() << std::endl;
    return;
  }

  mServerSocket =
      socket(serverResult->ai_family, serverResult->ai_socktype, serverResult->ai_protocol);
  if (mServerSocket == INVALID_SOCKET) {
    std::cerr << "creating socket failed with " <<
                             WSAGetLastError();
  }

  if (ioctlsocket(mServerSocket,
      FIONBIO,
      &uNonBlockingMode
  ) != 0) ERR("ioctlsocket");




  mIResult = bind(mServerSocket, serverResult->ai_addr, (int)serverResult->ai_addrlen);
  // resultMessage("binding ", m_iResult);

  freeaddrinfo(serverResult);

  mIResult = listen(mServerSocket, SOMAXCONN);
  if (mIResult != 0) { ERR("listen"); }
  events[0].fd = mServerSocket;
  events[0].events = POLLIN;
  int event_count = 1; // 1 because of the server socket
  std::cout << "Listening on " << DEFAULT_PORT << std::endl;
  while (true) {

      int pollResult = WSAPoll(events, MAX_EVENTS + 1, 0);
      if (pollResult) {
          if (events[0].revents & POLLIN) {
              mClientSocket = accept(mServerSocket, NULL, NULL);
              if (mClientSocket == INVALID_SOCKET) {
                  continue;
              }
              ioctlsocket(mClientSocket, FIONBIO, &uNonBlockingMode);
              if (event_count < MAX_EVENTS + 1) {
                  for (int i = 1; i < MAX_EVENTS; i++)
                  {
                      if (events[i].fd == 0 && events[i].events == 0)
                      {
                          events[i].fd = mClientSocket;
                          events[i].events = POLLIN;
                          event_count++;
                          break;
                      }
                  }
              }
              else { send(mClientSocket, "Server full", sizeof("Server full"), 0); } // Sorry I couldn't bother
          }
          for (int i = 1; i <= MAX_EVENTS; i++)
          {
              if (events[i].fd != 0 && events[i].revents & POLLIN)
              {
                  mBytesReceived = recv(events[i].fd, mRecvBuf, DEFAULT_BUFLEN - 1, 0);
                  mRecvBuf[DEFAULT_BUFLEN - 1] = '\0';
                  std::string fullRequest{};
                  fullRequest.append(mRecvBuf);
                  if (fullRequest.find("\r\n\r\n") != std::string::npos)
                  {
                      Server::ReturnStatus parsingResult = ParseRequest(fullRequest);
                      std::string buffer = SendMessage(parsingResult);
                      if(send(events[i].fd, buffer.c_str(), buffer.size(), 0)==SOCKET_ERROR)ERR("send");
                  }
                  if (mBytesReceived == 0)
                  {
                      if (shutdown(events[i].fd, SD_SEND) != 0) ERR("shutdown");
                      closesocket(events[i].fd);
                      events[i].fd = 0;
                      events[i].events = 0;
                      event_count--;
                  }
                  

              }
          }


          
      }
  }
}
