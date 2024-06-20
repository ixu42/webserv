#include "Server.hpp"

int main()
{
	try
	{
		throw ServerException("dummy exception from main");
		// Server testServer;
		// Server testServer("10.11.5.17", 6500);
		// Server testServer("127.127.3.1", 80);
		Server testServer("", 6500);
	}
	catch (const ServerException& e)
	{
		std::cout << "Message: " << e.what() << std::endl;
	}

	return 0;
}