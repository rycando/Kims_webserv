#include "./includes/Communicate.hpp"
#include "./includes/Config.hpp"
#include "./includes/Handler.hpp"
#include "./includes/Server.hpp"
#include "./includes/utils.hpp"

std::vector<Server>		g_servers;

int main(int argc, char** argv)
{
	Server	server;
	Config	config;
	Handler handler;
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
		// for (std::vector<Server>::iterator it = g_servers.begin(); it != g_servers.end(); it++)
		// {
		// 	for (unsigned long i = 0; i < it->_conf.size(); i++)
		// 	{
		// 		show_config(it->_conf.at(i));
		// 	}
		// }

		initialize_fdsets(&rSet, &wSet, &readSet, &writeSet, &timeout);

		for (std::vector<Server>::iterator it(g_servers.begin()); it != g_servers.end(); ++it)
		{
			it->init(&readSet, &writeSet, &rSet, &wSet);
		}
		ft::logger("Initializing g_servers completed\n", 1);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	
	while (g_state)
	{
		FD_COPY(&rSet, &readSet);
		FD_COPY(&wSet, &writeSet);

		int ret;
		ret = select(config.getMaxFd(g_servers) + 1, &readSet, &writeSet, NULL, &timeout);

		int i = 3;
		std::string fds;
		while (FD_ISSET(i, &readSet))
		{
			fds += i;
			std::cout << i << " ";
			i++;
			if (!FD_ISSET(i, &readSet))
			{
				std::cout << std::endl;
			}
		}
		for (std::vector<Server>::iterator server(g_servers.begin()); server != g_servers.end(); server++)
		{
			if (!g_state)
				break ;
			try
			{
				server->connect(getOpenFd(g_servers), &readSet);
			}
			catch(const std::exception& e)
			{
				std::cerr << e.what() << std::endl;
			}
			if (!server->_tmp_clients.empty())
				handler.send503(server->getWSet(), server->_tmp_clients);

			for (std::vector<Client *>::iterator c(server->_clients.begin()); c != server->_clients.end(); c++)
			{
				if (!(communicate(&readSet, &writeSet, *server, *c)))
				{
					ft::logger(("communicate return 0 with client fd : " + std::to_string((*c)->_fd) + " read_fd : " + std::to_string((*c)->_read_fd)), 1);
					if (*c)
						delete *c;
					server->_clients.erase(c);
					break ;
				}
			}
		}
	}
	g_servers.clear();
	return (0);
}
