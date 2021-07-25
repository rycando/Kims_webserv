#include "./includes/Server.hpp"
#include "./includes/Config.hpp"
#include "./includes/Client.hpp"
#include <iostream>

std::vector<Server>		g_servers;

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

int main(int argc, char** argv)
{
	Server	server;
	Config	config;
	// Client	*client;

	fd_set	readSet;
	fd_set	writeSet;
	fd_set	rSet;
	fd_set	wSet;
	bool	g_state = true;
	struct timeval timeout;


	std::string tmp;
	
	if (argc != 2)
		return (0);
	try 
	{
		config.parse(argv[1], g_servers);

		initialize_fdsets(&rSet, &wSet, &readSet, &writeSet, &timeout);

		for (std::vector<Server>::iterator it(g_servers.begin()); it != g_servers.end(); ++it)
		{
			it->init(&rSet, &wSet, &readSet, &writeSet);
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	
	while (g_state)
	{
		readSet = rSet;
		writeSet = wSet;
		
		select(config.getMaxFd(g_servers) + 1, &readSet, &writeSet, NULL, &timeout);
		
		for (std::vector<Server>::iterator server(g_servers.begin()); server != g_servers.end(); server++)
		{
			if (!g_state)
				break ;
			try
			{
				server->connect(getOpenFd(g_servers));
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
			
			if (!server->_tmp_clients.empty())
				server->send503(server->_tmp_clients.front());
			
			for (std::vector<Client *>::iterator c(server->_clients.begin()); c != server->_clients.end(); c++)
			{
				if (!((*c)->communicate(&readSet, &writeSet, *server)))
					break ;
			}
		}
	}
	g_servers.clear();
	return (0);
}
