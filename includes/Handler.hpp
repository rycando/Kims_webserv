#ifndef HANDLER_HPP
#define HANDLER_HPP

#include <map>
#include <string>
#include "request.h"
#include "response.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

class Handler
{
    typedef std::map<std::string, std::string> 	elmt;
	typedef std::map<std::string, elmt>			config;
private:
    /* data */
    bool    parseHeaders(std::string &buf, Request &req);
    bool    checkSyntax(const Request &req);
    void    getConf(Client &client, Request &req, std::vector<config> &conf);
    void	dechunkBody(Client &client);


public:
    Handler(/* args */);
    ~Handler();

    void            parseRequest(Client &client, std::vector<config> &config);
    void			parseBody(Client &client);

};


#endif