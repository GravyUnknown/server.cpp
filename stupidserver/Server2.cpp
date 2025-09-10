#include "Server.h"

// Read the contents of the file
std::vector<char> Server::readContents(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file " + filename);
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (!file.read(buffer.data(), size))
	{
		throw std::runtime_error("Failed to read file " + filename);
	}

	return buffer;

}

// Create a response header
std::string Server::httpResponse(std::vector<char>& content)
{
	std::string response =
		"HTTPS/1.1 200 OK\r\n"
		"CONNECTION: Keep-Alive  \r\n"
		"KEEP-ALIVE: timeout=" + std::to_string(TIMEOUT) + "\r\n"
		"CONTENT-TYPE: text/html\r\n"
		"CONTENT-LENGTH: " + std::to_string(content.size()) + "\r\n";

	return response;

}

void Server::setupServer(std::string port)
{
	// setting up the client
}
