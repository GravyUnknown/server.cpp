#undef UNICODE

#define WIN32_LEAN_AND_MEAN

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

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Server {
private:
  WSADATA wsadata;
  SOCKET mServerSocket = INVALID_SOCKET;
  SOCKET mClientSocket = INVALID_SOCKET;
  std::string mBuffer;
  int mIResult;
  int mBytesReceived;
  char mRecvBuf[DEFAULT_BUFLEN];
  struct addrinfo *result = NULL;
  struct addrinfo hints;
  struct addrinfo *p;
  enum class Status { ERR_BADREQUEST, ERR_INVALIDMETHOD, ERR_NOTFOUND, OK };
  struct ReturnStatus {
    Status returnCode;
    std::string route;
  };

public:
  ~Server();
  bool ResultMessage(const std::string &error, int result);
  std::string ReadContents(const std::string &filename);
  ReturnStatus ParseRequest(const std::string &recvBuf);
  int SendMessage();
  void InitializeWinsock();
  void SetupServer();
};
