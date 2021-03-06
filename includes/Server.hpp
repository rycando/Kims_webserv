#ifndef Server_HPP
# define Server_HPP

#define PORT 9000
#define TIMEOUT 10

#include "Webserv.hpp"
#include "Handler.hpp"
#include "Client.hpp"
#include "utils.hpp"

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
		Handler					_handler;

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
		void	connect(int openFd, fd_set *readSet);
		void	holdConnection();
		void	acceptConnection();
		void	send503(int fd);
		int		readRequest(Client *client);
		int		writeResponse(Client *client);
		int		getTimeDiff(std::string start);
		fd_set *getWSet();

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
