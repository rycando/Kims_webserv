#include "../includes/Server.hpp"

Server::Server() :  _fd(-1), _maxFd(-1), _port(-1)
{
	memset(&_s_addr, 0, sizeof(_s_addr));
}

Server::~Server()
{
	Client		*client = NULL;

	if (_fd != -1)
	{
		for (std::vector<Client*>::iterator it(_clients.begin()); it != _clients.end(); ++it)
		{
			client = *it;
			*it = NULL;
			if (client)
				delete client;
		}
		while (!_tmp_clients.empty())
		{
			close(_tmp_clients.front());
			_tmp_clients.pop();
		}
		_clients.clear();
		close(_fd);
		FD_CLR(_fd, _rSet);
	}
}

int		Server::getFd() const
{
	return (this->_fd);
}

int		Server::getMaxFd()
{
	Client	*client;

	for (std::vector<Client*>::iterator it(_clients.begin()); it != _clients.end(); ++it)
	{
		client = *it;
		if (client->_read_fd > _maxFd)
			_maxFd = client->_read_fd;
		if (client->_write_fd > _maxFd)
			_maxFd = client->_write_fd;
	}
	return (this->_maxFd);
}

void Server::init(fd_set *readSet, fd_set *writeSet, fd_set *rSet, fd_set *wSet)
{
	int					option = 1;
	std::string			parse_address;
	std::string			host;
	int					c_idx;
	
	_readSet = readSet;
	_writeSet = writeSet;
	_wSet = wSet;
	_rSet = rSet;

	parse_address = _conf[0]["server|"]["listen"];

	//1. 소켓 생성
	if ((_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		throw (ServerException("socket() error", std::string(strerror(errno))));

	// 소켓이 wait 상태에서도 포트를 할당할 수 있게 SO_REUSEADDR로 socket을 set 해 준다.
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
		throw (ServerException("setsockopt() error", std::string(strerror(errno))));

	memset(&_s_addr, 0, sizeof(_s_addr));

	//2. 연결 요청을 수신할 주소 설정 - _conf에서 host와 port를 가져오자.
	if ((const unsigned long)(c_idx = parse_address.find(":")) != std::string::npos)
	{
		host = parse_address.substr(0, c_idx);
		if ((_port = atoi(parse_address.substr(c_idx + 1).c_str())) < 0)   // client port
			throw (ServerException("Wrong port", std::to_string(_port)));
		_s_addr.sin_addr.s_addr = inet_addr(host.c_str()); 		// inet_addr : 문자열 주소를 32비트 정수로 변환
	}
	else
	{
		_s_addr.sin_addr.s_addr = INADDR_ANY;  // INADDR_ANY : 0으로 define된 값으로, 사용가능한 LAN 카드의 IP주소를 사용하라는 의미
		if ((_port = atoi(parse_address.c_str())) < 0)  // client port
			throw (ServerException("Wrong port", std::to_string(_port)));
	}
	_s_addr.sin_port = htons(_port); 		// htonl : long형 호스트 바이트 순서를 네트워크 바이트 순서로 변환
	_s_addr.sin_family = AF_INET;			// AF_INET : IPv4 인터넷 프로토콜에 적용하는 주소체계
	
	//3. 소켓을 포트에 연결
	if (bind(_fd, (struct sockaddr *)&_s_addr, sizeof(_s_addr)) == -1) 
		throw (ServerException("bind()", std::string(strerror(errno))));
	
	//4. 커널에 개통 요청
	if (listen(_fd, 5) == -1)
		throw (ServerException("listen()", std::string(strerror(errno))));

	// _fd를 non-blocking 모드로 바꿈
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1)
		throw (ServerException("fcntl()", std::string(strerror(errno))));

	FD_SET(_fd, _rSet);
	_maxFd = _fd;
}

void Server::connect(int openFd, fd_set *Set)
{
	int					MAX_FD = 256 - 20;
	int					ret;

	if ((ret = FD_ISSET(_fd, Set)))
	{
		if (openFd > MAX_FD)
			holdConnection();
		else
			acceptConnection();
	}
}

void	Server::holdConnection()
{
	int fd;
	struct sockaddr_in c_addr;
	socklen_t len;
	
	len = sizeof(struct sockaddr);
	if ((fd = accept(_fd, (struct sockaddr *)&c_addr, &len)) == -1)
		throw (ServerException("accept() error in holdConnection(): ", std::string(strerror(errno))));

	if (_tmp_clients.size() < 10)
	{
		_tmp_clients.push(fd);
		FD_SET(fd, _wSet);
	}
	else
		close(fd);
}

void	Server::acceptConnection()
{
	int fd;
	struct sockaddr_in c_addr;
	socklen_t len;
	Client *client;
	
	len = sizeof(struct sockaddr);
	memset(&c_addr, 0, len);
	
	//5. 클라이언트 연결 요청 수신
	if ((fd = accept(_fd, (struct sockaddr *)&c_addr, &len)) == -1)
		throw (ServerException("accept() error in acceptConnection(): ", std::string(strerror(errno))));
	if (fd > _maxFd)
		_maxFd = fd;
	client = new Client(fd, _rSet, _wSet, c_addr);
	_clients.push_back(client);
}

int		Server::getOpenFd()
{
	int 	openFd = 0;
	Client	*client;

	for (std::vector<Client*>::iterator it(_clients.begin()); it != _clients.end(); ++it)
	{
		client = *it;
		openFd += 1;  // socket 함수로 클라이언트를 생성할때 생기는 fd(+1)
		if (client->_read_fd != -1)  // 클라이언트의 read_fd set 이면
			openFd += 1;
		if (client->_write_fd != -1) // 클라이언트의 write_fd set 이면
			openFd += 1;
	}
	openFd += _tmp_clients.size();  // 대기중인 클라이언트 수만큼 증가
	return (openFd);
}


int				Server::readRequest(Client *client)
{
	int			length;
	int			readed;
	std::string log;

	length = strlen(client->_buf);
	readed = read(client->_fd, client->_buf + length, BUFFER_SIZE - length);
	length += readed;
	if (readed > 0)
	{
		client->_buf[length] = '\0';
		if (client->_status != Client::BODYPARSING)
		{
			log = "REQUEST:\n";
			log += client->_buf;
			client->_last_date = ft::getDate();
			_handler.parseRequest(*client, _conf);
			client->setWriteState(true);
		}
		else
			_handler.parseBody(*client);
		return (1);
	}
	return (0);
}

int					Server::writeResponse(Client *client)
{
	unsigned long	length;
	std::string		log;

	switch (client->_status)
	{
		case Client::RESPONSE:
			length = write(client->_fd, client->_response.c_str(), client->_response.size());
			if (length < client->_response.size())
				client->_response = client->_response.substr(length);
			else
			{
				client->_response.clear();
				client->setToStandBy();
			}
			client->_last_date = ft::getDate();
			break;
		case Client::STANDBY:
			if (getTimeDiff(client->_last_date) >= TIMEOUT) 
				client->_status = Client::DONE;
			break ;
		case Client::DONE:
			ft::logger("[" + std::to_string(_port) + "] " + "connected clients: " + std::to_string(_clients.size()), 1);
			return (0);
		default:
			_handler.dispatcher(*client);
			ft::logger("Client : " + client->client_status[client->_status], 0);
	}
	return (1);
}

Server::ServerException::ServerException(void)
{
	this->error = "Undefined Server Exception";
}

Server::ServerException::ServerException(std::string function, std::string error)
{
	this->error = function + ": " + error;
}

Server::ServerException::~ServerException(void) throw() {}

const char			*Server::ServerException::what(void) const throw()
{
	return (this->error.c_str());
}


fd_set	*Server::getWSet(){
	return _wSet;
}


int		Server::getTimeDiff(std::string start)
{
	struct tm		start_tm;
	struct tm		*now_tm;
	struct timeval	time;
	int				result;

	strptime(start.c_str(), "%a, %d %b %Y %T", &start_tm);
	gettimeofday(&time, NULL);
	now_tm = localtime(&time.tv_sec);
	result = (now_tm->tm_hour - start_tm.tm_hour) * 3600;
	result += (now_tm->tm_min - start_tm.tm_min) * 60;
	result += (now_tm->tm_sec - start_tm.tm_sec);
	return (result);
}
