#include "../includes/Handler.hpp"

void	Handler::dispatcher(Client &client)
{
	typedef void	(Handler::*ptr)(Client &client);
	std::map<std::string, ptr> map;

	map["GET"] = &Handler::handleGet;
	// map["POST"] = &Handler::handlePost;
	// map["DELETE"] = &Handler::handleDelete;

	(this->*map[client._req.method])(client);
}

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

void	Handler::handleGet(Client &client)
{
	struct stat	file_info;

	switch (client._status)
	{
		case Client::CODE:
			_helper.getStatusCode(client);
			fstat(client._read_fd, &file_info);
			if (S_ISDIR(file_info.st_mode) && client._conf["listing"] == "on")
				createListing(client);
			if (client._res.status_code == NOTFOUND)
				negotiate(client);
			if (((client._conf.find("CGI") != client._conf.end() && client._req.uri.find(client._conf["CGI"]) != std::string::npos)
			|| (client._conf.find("php") != client._conf.end() && client._req.uri.find(".php") != std::string::npos))
			&& client._res.status_code == OK)
			{
				execCGI(client);
				client._status = Client::CGI;
			}
			else
				client._status = Client::HEADERS;
			client.setFileToRead(true);	
			break ;
		case Client::CGI:
			if (client._read_fd == -1)
			{
				parseCGIResult(client);
				client._status = Client::HEADERS;
			}
			break ;
		case Client::HEADERS:
			lstat(client._conf["path"].c_str(), &file_info);
			if (!S_ISDIR(file_info.st_mode))
				client._res.headers["Last-Modified"] = _helper.getLastModified(client._conf["path"]);
			if (client._res.headers["Content-Type"][0] == '\0')
				client._res.headers["Content-Type"] = _helper.findType(client);
			if (client._res.status_code == UNAUTHORIZED)
				client._res.headers["WWW-Authenticate"] = "Basic";
			else if (client._res.status_code == NOTALLOWED)
				client._res.headers["Allow"] = client._conf["methods"];
			client._res.headers["Date"] = ft::getDate();
			client._res.headers["Server"] = "webserv";
			client._status = Client::BODY;
			break ;
		case Client::BODY:
			if (client._read_fd == -1)
			{
				client._res.headers["Content-Length"] = std::to_string(client._res.body.size());
				createResponse(client);
				client._status = Client::RESPONSE;
			}
			break;
	}
}