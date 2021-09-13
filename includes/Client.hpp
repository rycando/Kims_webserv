#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Webserv.hpp"
#include "HttpMessage.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 32768
#define TMP_PATH 	"/tmp/cgi.tmp"

class Client
{
	friend class Server;
	friend class Helper;
	friend class Handler;
	
	typedef std::map<std::string, std::string> t_conf;
	struct t_chunk
	{
		unsigned int	len;
		bool			done;
		bool			found;
	};

	private:
		int				_port;
		int				_status;
		int				_cgi_pid;
		int				_tmp_fd;
		char 			*_buf;
		fd_set			*_rSet;
		fd_set			*_wSet;
		std::string		_ip;
		std::string		_last_date;
		std::string		_response;
		t_conf 			_conf;
		t_chunk			_chunk;
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

		std::string client_status[9];

		int		_fd;
		int		_read_fd;
		int		_write_fd;
		void	readFile();
		void	writeFile();

		void	setReadState(bool status);
		void	setWriteState(bool status);
		void	setFileToRead(bool status);
		void	setFileToWrite(bool status);
		void	setToStandBy();
};
	

#endif
