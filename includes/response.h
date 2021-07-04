#ifndef RESPONSE_H
#define RESPONSE_H

#include <map>
#include <string>

struct Response
{
    std::string							version;
    std::string							status_code;
    std::map<std::string, std::string>	headers;
	std::string							body;
};


#endif