#include "Server.hpp"

int main() {
  Server server;
  server.InitializeWinsock();
  server.SetupServer();
}
