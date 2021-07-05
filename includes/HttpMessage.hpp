#ifndef HTTPMESSAGE_HPP
# define HTTPMESSAGE_HPP

#include <string>
#include <map>

class Request
{
public:
	Request()
	{
	}
	~Request()
	{
	}
	
	std::string method;
	std::string uri;
	std::map<std::string, std::string> headers;
	std::string body;
	
	void clear()
	{
		method.clear();
		uri.clear();
		headers.clear();
		body.clear();
	}
};

class Response
{
public:

	Response()
	{
	}
	~Response()
	{
	}

	std::string version;
	std::string status_code;
	std::map<std::string, std::string> headers;
	std::string	body;
	
	void clear()
	{
		version.clear();
		status_code.clear();
		headers.clear();
		body.clear();
	}	
};

#endif
