#include "../includes/Client.hpp"

Client::Client(int fd, fd_set *r, fd_set *w, struct sockaddr_in c_addr)
 : _cgi_pid(-1), _tmp_fd(-1), _rSet(r), _wSet(w), _fd(fd), _read_fd(-1), _write_fd(-1)
{
	_status = STANDBY;
	_ip = inet_ntoa(c_addr.sin_addr);
	_port = htons(c_addr.sin_port);
	_buf = (char *)malloc(sizeof(char) * BUFFER_SIZE + 1);
	memset(_buf, 0, BUFFER_SIZE + 1);
	fcntl(_fd, F_SETFL, O_NONBLOCK);
	FD_SET(_fd, _rSet);
	FD_SET(_fd, _wSet);
	_last_date = ft::getDate();
	client_status[0] = "PARSING";
	client_status[1] = "BODYPARSING";
	client_status[2] = "CODE";
	client_status[3] = "HEADERS";
	client_status[4] = "CGI";
	client_status[5] = "BODY";
	client_status[6] = "RESPONSE";
	client_status[7] = "STANDBY";
	client_status[8] = "DONE";
	ft::logger("new connection from " + _ip + ":" + std::to_string(_port), 1);
}

Client::~Client()
{
	free(_buf);
	_buf = NULL;
	if (_fd != -1)
	{
		close(_fd);
		FD_CLR(_fd, _rSet);
		FD_CLR(_fd, _wSet);
	}
	if (_read_fd != -1)
	{
		close(_read_fd);
		FD_CLR(_read_fd, _rSet);
		FD_CLR(_read_fd, _wSet);
	}
	if (_write_fd != -1)
	{
		close(_write_fd);
		FD_CLR(_write_fd, _rSet);
		FD_CLR(_write_fd, _wSet);
	}
	if (_tmp_fd != -1)
	{
		close(_tmp_fd);
		FD_CLR(_tmp_fd, _rSet);
		FD_CLR(_tmp_fd, _wSet);		
	}
	ft::logger("connection closed from " + _ip + ":" + std::to_string(_port), 1);
}

void	Client::setFileToRead(bool state)
{
	if (_read_fd != -1)
	{
		if (state)
			FD_SET(_read_fd, _rSet);
		else
			FD_CLR(_read_fd, _rSet);
	}
}

void	Client::setFileToWrite(bool status)
{
	if (_write_fd != -1)
	{
		if (status)
			FD_SET(_write_fd, _wSet);
		else
			FD_CLR(_write_fd, _wSet);
	}
}

void	Client::setReadState(bool status)
{
	if (status)
		FD_SET(_fd, _rSet);
	else
		FD_CLR(_fd, _rSet);
}


void	Client::setWriteState(bool status)
{
	if (status)
		FD_SET(_fd, _wSet);
	else
		FD_CLR(_fd, _wSet);
}

void	Client::readFile()
{
	int ret = 0;
	int	status = 0;
	char buffer[BUFFER_SIZE + 1];

	if (_cgi_pid != -1)
	{
		if (!waitpid((pid_t)_cgi_pid, &status, WNOHANG))
			return ;
		else 
		{
			if (WEXITSTATUS(status) == 1)
			{
				close(_tmp_fd);
				_tmp_fd = -1;
				_cgi_pid = -1;
				close(_read_fd);
				setFileToRead(false);
				_read_fd = -1;
				_res.body = "Error with cgi\n";
				return;
			}
		}
	}
	
	try 
	{
		ret = read(_read_fd, buffer, BUFFER_SIZE);
		buffer[ret] = '\0';
		std::string tmp(buffer, ret);
		_res.body += tmp;
		if (ret == 0)
		{
			close(_read_fd);
			setFileToRead(false);
			_read_fd = -1;
		}	
	}
	catch (std::exception &e)
	{
		std::cerr << "ReadFile() error\n" << e.what() << std::endl;
	}
}

void	Client::writeFile()
{
	int write_size;
	
	write_size = write(_write_fd, _req.body.c_str(), _req.body.size());
	if (_cgi_pid != -1)
		ft::logger("sent " + std::to_string(write_size) + " bytes to CGI stdin", 1);
	if ((unsigned long)write_size < _req.body.size())
		_req.body = _req.body.substr(write_size);
	else
	{
		_req.body.clear();
		close(_write_fd);
		setFileToWrite(false);
		_write_fd = -1;
	}
}

void	Client::setToStandBy()
{

	ft::logger(_req.method + " from " + _ip + ":" + std::to_string(_port) + " answered", _port);
	_status = STANDBY;
	setReadState(true);
	if (_read_fd != -1)
		close(_read_fd);
	if (_write_fd != -1)
		close(_write_fd);
	_read_fd = -1;
	_write_fd = -1;
	memset((void *)_buf, 0, (size_t)(BUFFER_SIZE + 1));
	_conf.clear(); 
	_req.clear();
	_res.clear();
}
