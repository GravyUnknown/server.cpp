#include "Server.hpp"

int main()
{
	Server server;
	server.initializeWinsock();
	server.setupServer();
}