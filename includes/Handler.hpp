#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "Webserv.hpp"
#include "Helper.hpp"
#include "HttpMessage.hpp"
#include "utils.hpp"

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
    void	createListing(Client &client);
    void	negotiate(Client &client);
    void    getBody(Client &client);
    void    execCGI(Client &client);
    void	parseCGIResult(Client &client);

    void    handleGet(Client &client);
    void    handlePost(Client &client);
    void    handleDelete(Client &client);

    void    postInCode(Client &client);
    void    postInCGI(Client &client);
    void    postInHeaders(Client &client);
    void    postInBody(Client &client);

    void    getInCode(Client &client, struct stat *file_info);
    void    getInCGI(Client &client);
    void    getInHeaders(Client &client, struct stat *file_info);
    void    getInBody(Client &client);

    void    deleteInCode(Client &client);
    void    deleteInBody(Client &client);


public:
    Handler(/* args */);
    ~Handler();

    Helper	_helper;

    void    parseRequest(Client &client, std::vector<config> &config);
    void    parseBody(Client &client);
    void	createResponse(Client &client);
    void    dispatcher(Client &client);
    void    send503(fd_set *wSet, std::queue<int> &tmp_clients);

};


#endif