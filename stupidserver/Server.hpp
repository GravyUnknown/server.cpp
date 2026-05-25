#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <filesystem>
#include <fstream>
#include <iostream>
#include <istream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#undef SendMessage

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")



inline int constexpr DEFAULT_BUFLEN = 512;
inline std::string_view constexpr DEFAULT_PORT = "27015";
inline int constexpr MAX_EVENTS = 10;


class Server {
private:
  WSADATA wsadata;
  SOCKET mServerSocket = INVALID_SOCKET;
  SOCKET mClientSocket = INVALID_SOCKET;
  WSAPOLLFD events[MAX_EVENTS + 1];
  std::string mBuffer;
  int mIResult;
  int mBytesReceived;
  char mRecvBuf[DEFAULT_BUFLEN];
  struct addrinfo *serverResult = NULL;
  struct addrinfo hints;
  struct addrinfo *p;
  enum class Status { UNKNOWN, ERR_BADREQUEST, ERR_INVALIDMETHOD, ERR_NOTFOUND, OK };
  struct ReturnStatus {
    Status returnCode = Status::UNKNOWN;
    std::string route{};
  };
  static std::string indexHTML, aboutHTML;
  ULONG uNonBlockingMode = 1;
  inline void ERR(auto e) { std::cout << e << " failed with" << WSAGetLastError(); };

public:
   
  ~Server();
  bool ResultMessage(const std::string &error, int result);
  std::string ReadContents(const std::filesystem::path &filename);
  ReturnStatus ParseRequest(const std::string &recvBuf);
  std::string SendMessage(Server::ReturnStatus& presult);
  void InitializeWinsock();
  void SetupServer();
};
