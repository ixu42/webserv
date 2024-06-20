#include <map>
#include <string>

class RequestHandler
{
	typedef std::map<std::string, std::string> RequestMessage;
	private:
		RequestMessage requestMessage;
		std::string responseMessage;

	public:
		RequestMessage parse(std::string request);
		std::string handleRequest();

};


// GET / HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// Cache-Control: max-age=0
// sec-ch-ua: "Not/A)Brand";v="8", "Chromium";v="126", "Google Chrome";v="126"
// sec-ch-ua-mobile: ?0
// sec-ch-ua-platform: "macOS"
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br, zstd
// Accept-Language: en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7
// Cookie: wp-settings-1=libraryContent%3Dbrowse%26posts_list_mode%3Dlist; wp-settings-time-1=1697667275; adminer_permanent=c2VydmVy--cm9vdA%3D%3D-bG9jYWw%3D%3AWdDaEmjuEAY%3D

