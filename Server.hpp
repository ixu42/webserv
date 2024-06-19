#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <string>

#include <unistd.h>

class Server
{
	private:

		int					serverSocket;
		int					domain = AF_INET;
		__socket_type		type = SOCK_STREAM;
		int					protocol = 0;
		in_addr_t			address = INADDR_ANY;
		int					port = 8080;
		struct sockaddr_in	sockAddress;

		int					newSocket;

	public:
		Server();

		void createSocket();
		void bindSocket();
		void listenConnection();
		void acceptConnection();
};
