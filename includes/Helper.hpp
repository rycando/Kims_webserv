#ifndef HELPER_HPP
#define HELPER_HPP

#include "utils.h"
#include "Client.hpp"

class Helper
{
private:
    /* data */
public:
	int				findLen(Client &client);
	int				fromHexa(const char *nb);
    void		    fillBody(Client &client);
    void			parseAccept(Client &client, std::multimap<std::string, std::string> &map, std::string header);



    Helper(/* args */);
    ~Helper();
};

#endif