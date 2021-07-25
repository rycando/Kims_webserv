#include "./includes/Server.hpp"
#include "./includes/Config.hpp"
#include "./includes/Client.hpp"
#include "./includes/Handler.hpp"
#include <iostream>
#include <sys/select.h>

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

int	communicate(fd_set *readSet, fd_set *writeSet, Server &server, Client *client)
{
	if (FD_ISSET(client->_fd, readSet))  // 클라이언트가 서버로 데이터를 전달할 때
		if (!server.readRequest(client))  // 클라이언트가 보낸 request 메시지를 읽고 파싱. 다읽었다면 0을 반환하여 반복문 탈출.
			return (0);
	if (FD_ISSET(client->_fd, writeSet))
		if (!server.writeResponse(client))
			return (0);
	if (client->_write_fd != -1)
		if (FD_ISSET(client->_write_fd, writeSet))
			client->writeFile();
	if (client->_read_fd != -1)
		if (FD_ISSET(client->_read_fd, readSet))
			client->readFile(); // 에러메세지도 들어옴 getErrorPage()
	return (1);
}


int main(int argc, char** argv)
{
	Server	server;
	Config	config;
	Handler handler;
	Client	*client;
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
				handler.send503(server->getWSet(), server->_tmp_clients);
			
			for (std::vector<Client *>::iterator c(server->_clients.begin()); c != server->_clients.end(); c++)
			{
				if (!(communicate(&readSet, &writeSet, *server, *c)))
					break ;
			}
		}
	}
	g_servers.clear();
	return (0);
}
