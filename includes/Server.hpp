#ifndef Server_HPP
# define Server_HPP

#define PORT 9000

#include <queue>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/errno.h>

#include "Client.hpp"

class Client;

class Server
{
	friend class Config;
	typedef std::map<std::string, std::string> 	elmt;
	typedef std::map<std::string, elmt>			config;

	private:

		int						_fd;
		int						_maxFd;
		int						_port;
		struct sockaddr_in		_s_addr;
		fd_set					*_readSet;
		fd_set					*_writeSet;
		fd_set					*_rSet;
		fd_set					*_wSet;
		// Handler					_handler;

	public:
	
		std::vector<config>		_conf;		


		std::vector<Client*>	_clients;
		std::queue<int>			_tmp_clients;

		Server();
		~Server();
		void	init(fd_set *readSet, fd_set *writeSet, fd_set *rSet, fd_set *wSet);
		int		getFd() const;
		int		getMaxFd();
		int		getOpenFd();
		void	connect(int openFd);
		void	holdConnection();
		void	acceptConnection();
		void	send503(int fd);
		int		readRequest(Client *client);
		int		writeResponse(Client *client);

	class		ServerException: public std::exception
	{
		private:
			std::string function;
			std::string error;

		public:
			ServerException(void);
			ServerException(std::string function, std::string error);
			virtual	~ServerException(void) throw();	
			virtual const char *what(void) const throw();
	};
};

#endif
