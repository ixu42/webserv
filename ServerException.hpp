#include <stdexcept>
#include <exception>

class ServerException : public std::runtime_error
{
	public:
		explicit ServerException(const std::string &message)
		: std::runtime_error(message) {}
};