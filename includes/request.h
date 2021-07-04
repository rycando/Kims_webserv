#ifndef REQUEST_H
#define REQUEST_H

#include <map>
#include <string>

struct Request
{
	bool								valid;
	std::string							method;
	std::string							uri;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
};


#endif