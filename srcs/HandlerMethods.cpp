#include "../includes/Handler.hpp"

void	Handler::send503(fd_set *wSet, std::queue<int> &tmp_clients)
{
	Response		response;
	std::string		str;
	int				ret = 0;
	int				fd;

	fd = tmp_clients.front();

	response.version = "HTTP/1.1";
	response.status_code = UNAVAILABLE;
	response.headers["Retry-After"] = "25";
	response.headers["Date"] = ft::getDate();
	response.headers["Server"] = "webserv";
	response.body = UNAVAILABLE;
	response.headers["Content-Length"] = std::to_string(response.body.size());
	std::map<std::string, std::string>::const_iterator b = response.headers.begin();
	str = response.version + " " + response.status_code + "\r\n";
	while (b != response.headers.end())
	{
		if (b->second != "")
			str += b->first + ": " + b->second + "\r\n";
		++b;
	}
	str += "\r\n";
	str += response.body;
	ret = write(fd, str.c_str(), str.size());
	if (ret >= -1)
	{
		close(fd);
		FD_CLR(fd, wSet);
		tmp_clients.pop();
	}
}
