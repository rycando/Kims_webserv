#ifndef COMMUNICATE_HPP
# define COMMUNICATE_HPP

#include "Webserv.hpp"
#include "Server.hpp"
#include "Client.hpp"

int		communicate(fd_set *readSet, fd_set *writeSet, Server &server, Client *client);
int		getOpenFd(std::vector<Server> &servers);
void	initialize_fdsets(fd_set *rSet, fd_set *wSet, fd_set *readSet, fd_set *writeSet, timeval *timeout);

#endif