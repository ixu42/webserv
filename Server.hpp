#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdexcept>
#include <exception>

#include <cstring>
#include <iostream>
#include <string>

#include <arpa/inet.h> // For inet_pton

#include <unistd.h>

#define DEFAULT_ADDRESS INADDR_ANY
#define DEFAULT_PORT	8090

class ServerException : public std::runtime_error
{
	public:
		explicit ServerException(const std::string &message)
		: std::runtime_error(message) {}
};

// class ServerException : public std::exception
// {
// 	private:
// 		std::string message;

// 	public:
// 		virtual const char* what() const noexcept;
// };

class Server
{
	private:

		int					serverSocket;

		const int			domain = AF_INET;
		__socket_type		type = SOCK_STREAM;
		int					protocol = 0;

		in_addr_t			address = DEFAULT_ADDRESS;
		int					port = DEFAULT_PORT;
		struct sockaddr_in	sockAddress;

		int					newSocket;

	public:
		Server();
		Server(std::string address, int port);

		void initialize();
		void createSocket();
		void bindSocket();
		void listenConnection();
		void acceptConnection();
};
