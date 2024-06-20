#include "ServerManager.hpp"
#include "Server.hpp"

int main()
{
	{
		ServerManager manager;
		manager.run();
	}



/* 	try
	{
		// throw ServerException("dummy exception from main"); // throws exception
		Server testServer;
		// Server testServer("10.11.5.17", 6500);
		// Server testServer("127.127.3.1", 80);
		// Server testServer("123.123.123.123", 6500); // throws exception
		// Server testServer("", 6500); // throws exception
	}
	catch (const ServerException& e)
	{
		std::cout << "Message: " << e.what() << std::endl;
	}

	return 0; */
}