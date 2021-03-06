#include "Communicate.hpp"

void initialize_fdsets(fd_set *rSet, fd_set *wSet, fd_set *readSet, fd_set *writeSet, timeval *timeout)
{
	signal(SIGINT, exit);
	FD_ZERO(readSet);
	FD_ZERO(writeSet);
	FD_ZERO(wSet);
	FD_ZERO(rSet);
	timeout->tv_sec = 1;
	timeout->tv_usec = 0;
}

int getOpenFd(std::vector<Server> &servers)
{
	int openFd = 0;
	
	for (std::vector<Server>::iterator server(servers.begin()); server != servers.end(); server++)
	{
		openFd++;
		openFd += server->getOpenFd();
	}
	return (openFd);
}

int	communicate(fd_set *readSet, fd_set *writeSet, Server &server, Client *client)
{
	if ((FD_ISSET(client->_fd, readSet)) && !(server.readRequest(client)))
		return (0);
	if (FD_ISSET(client->_fd, writeSet) && (!server.writeResponse(client)))
		return (0);
	if ((client->_write_fd != -1) && (FD_ISSET(client->_write_fd, writeSet)))
		client->writeFile();
	if ((client->_read_fd != -1) && (FD_ISSET(client->_read_fd, readSet)))
		client->readFile();
	return (1);
}

void show_elmt(std::map<std::string, std::string> &map)
{
    for (std::map<std::string, std::string>::iterator it = map.begin(); it != map.end() ; it++)
    {
        std::cout << "$" <<it->first << "$\t$" << it->second << "$" << std::endl;
    }
}

void show_config(std::map<std::string, elmt> &map)
{
    std::cout << "===============show_config=====================\n";
    for (std::map<std::string, elmt>::iterator it = map.begin(); it != map.end() ; it++)
    {
        std::cout << "$" << it->first << "$\n";
        show_elmt(it->second);
		std::cout << "--------------------------------\n";
    }
    std::cout << "==============================================\n";
}