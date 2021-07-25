#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "./Server.hpp"
#include "./HttpMessage.hpp"
#include "./Handler.hpp"
#include <arpa/inet.h>
#include <iostream>

#define BUFFER_SIZE 32768

class Server;

class Client
{
	friend class Server;
	friend class Helper;
	friend class Handler;
	typedef std::map<std::string, std::string> t_conf;

	private:
		int				_port;
		int				_status;
		int				_cgi_pid;
		int				_tmp_fd;
		char			*_buf;
		fd_set			*_rSet;
		fd_set			*_wSet;
		std::string		_ip;
		std::string		_last_date;
		std::string		_response;
		t_conf 			_conf;
		// t_chunk		chunk;
		Request			_req;
		Response		_res;

	public:	
	
		Client(int filed, fd_set *r, fd_set *w, struct sockaddr_in info);
		~Client();
		
		enum status
		{
			PARSING,
			BODYPARSING,
			CODE,
			HEADERS,
			CGI,
			BODY,
			RESPONSE,
			STANDBY,
			DONE
		};

		int		_fd;
		int		_read_fd;
		int		_write_fd;
		void	readFile();
		void	writeFile();

		void	setReadState(bool state);
		void	setWriteState(bool state);
		void	setFileToRead(bool state);
		void	setFileToWrite(bool state);
		void	setToStandBy();
		
		int		communicate(fd_set *readSet, fd_set *writeSet, Server &server);
};
	

#endif
