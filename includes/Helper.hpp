#ifndef HELPER_HPP
#define HELPER_HPP

#include "Webserv.hpp"
#include "Status.hpp"
#include "Client.hpp"
#include "utils.hpp"
#include "Status.hpp"
#include "Handler.hpp"

class Helper
{
    friend class Helper;

private:
    /* data */
public:
    std::map<std::string, std::string> MIMETypes;

	int				findLen(Client &client);
	int				fromHexa(const char *nb);
    void		    fillBody(Client &client);
    void			parseAccept(Client &client, std::multimap<std::string, std::string> &map, std::string header);
    std::string		decode64(const char *data);
    void			getErrorPage(Client &client);
    char			**setEnv(Client &client);
    std::string		getLastModified(std::string path);
	std::string		findType(Client &client);
    void			assignMIME();



    Helper(/* args */);
    ~Helper();

    int				getStatusCode(Client &client);
    int				GETStatus(Client &client);
    int				POSTStatus(Client &client);
    int				DELETEStatus(Client &client);
};

#endif