#include "../includes/Handler.hpp"

void	Handler::dispatcher(Client &client)
{
	typedef void	(Handler::*ptr)(Client &client);
	std::map<std::string, ptr> map;

	map["GET"] = &Handler::handleGet;
	map["POST"] = &Handler::handlePost;
	map["DELETE"] = &Handler::handleDelete;
	map["BAD"] = &Handler::handleBadRequest;
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

void	Handler::getInCode(Client &client, struct stat *file_info)
{
	_helper.getStatusCode(client);
	fstat(client._read_fd, file_info);
	if (S_ISDIR((*file_info).st_mode) && client._conf["listing"] == "on")
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
}

void	Handler::getInCGI(Client &client)
{
	if (client._read_fd == -1)
	{
		parseCGIResult(client);
		client._status = Client::HEADERS;
	}
}

void	Handler::getInHeaders(Client &client, struct stat *file_info)
{
	lstat(client._conf["path"].c_str(), file_info);
	if (!S_ISDIR((*file_info).st_mode))
		client._res.headers[""] = _helper.getLastModified(client._conf["path"]);
	if (client._res.headers["Content-Type"][0] == '\0')
		client._res.headers["Content-Type"] = _helper.findType(client);
	if (client._res.status_code == UNAUTHORIZED)
		client._res.headers["WWW-Authenticate"] = "Basic";
	else if (client._res.status_code == NOTALLOWED)
		client._res.headers["Allow"] = client._conf["methods"];
	client._res.headers["Date"] = ft::getDate();
	client._res.headers["Server"] = "webserv";
	client._status = Client::BODY;
}

void	Handler::getInBody(Client &client)
{
	if (client._read_fd == -1)
	{
		client._res.headers["Content-Length"] = std::to_string(client._res.body.size());
		createResponse(client);
		client._status = Client::RESPONSE;
	}
}

void		Handler::postInCode(Client &client)
{
	_helper.getStatusCode(client);
	if (((client._conf.find("CGI") != client._conf.end() && client._req.uri.find(client._conf["CGI"]) != std::string::npos) ||
		(client._conf.find("php") != client._conf.end() && client._req.uri.find(".php") != std::string::npos))
		&& client._res.status_code == OK)
	{
		execCGI(client); // ????????? ???????????? ??????
		client._status = Client::CGI;
		client.setFileToRead(true);
	}
	else
	{
		if (client._res.status_code == OK)
			client.setFileToWrite(true);
		else
			client.setFileToRead(true);
		client._status = Client::HEADERS;
	}
}

void		Handler::postInCGI(Client &client)
{
	if (client._read_fd == -1)
	{
		parseCGIResult(client);
		client._status = Client::HEADERS;
	}
}

void		Handler::postInHeaders(Client &client)
{
	if (client._res.status_code == UNAUTHORIZED)
		client._res.headers["WWW-Authenticate"] = "Basic";
	else if (client._res.status_code == NOTALLOWED)
		client._res.headers["Allow"] = client._conf["methods"];
	client._res.headers["Date"] = ft::getDate();
	client._res.headers["Server"] = "webserv";
	if (client._res.status_code == OK && !((client._conf.find("CGI") != client._conf.end() && client._req.uri.find(client._conf["CGI"]) != std::string::npos)
		|| (client._conf.find("php") != client._conf.end() && client._req.uri.find(".php") != std::string::npos)))
		client._res.body = "File modified\n";
	client._status = Client::BODY;
}

void		Handler::postInBody(Client &client)
{
	if (client._read_fd == -1 && client._write_fd == -1)
	{
		if (client._res.headers["Content-Length"][0] == '\0')
			client._res.headers["Content-Length"] = std::to_string(client._res.body.size());
		createResponse(client);
		client._status = Client::RESPONSE;
	}
}

void	Handler::deleteInCode(Client &client)
{
	if (!_helper.getStatusCode(client))
		client.setFileToRead(true);
	client._res.headers["Date"] = ft::getDate();
	client._res.headers["Server"] = "webserv";
	if (client._res.status_code == OK)
	{
		unlink(client._conf["path"].c_str());				
		client._res.body = "File deleted\n";
	}
	else if (client._res.status_code == NOTALLOWED)
		client._res.headers["Allow"] = client._conf["methods"];
	else if (client._res.status_code == UNAUTHORIZED)
		client._res.headers["WWW-Authenticate"] = "Basic";
	client._status = Client::BODY;
}

void	Handler::deleteInBody(Client &client)
{
	if (client._read_fd == -1)
	{
		client._res.headers["Content-Length"] = std::to_string(client._res.body.size());
		createResponse(client);
		client._status = Client::RESPONSE;
	}
}

void	Handler::handleGet(Client &client)
{
	struct stat	file_info;

	switch (client._status)
	{
		case Client::CODE:
			getInCode(client, &file_info);
			break ;
		case Client::CGI:
			getInCGI(client);
			break ;
		case Client::HEADERS:
			getInHeaders(client, &file_info);
			break ;
		case Client::BODY:
			getInBody(client);
			break;
	}
}

void		Handler::handlePost(Client &client)
{
	switch (client._status)
	{
		case Client::BODYPARSING:
			parseBody(client);
			break;
		case Client::CODE:
			postInCode(client);
			break;
		case Client::CGI:
			postInCGI(client);
			break;
		case Client::HEADERS:
			postInHeaders(client);
			break ;
		case Client::BODY:
			postInBody(client);
			break ;
	}
}

void	Handler::handleDelete(Client &client)
{
	switch (client._status)
	{
		case Client::CODE:
			deleteInCode(client);
			break ;
		case Client::BODY:
			deleteInBody(client);
			break ;
	}
}

void	Handler::handleBadRequest(Client &client)
{
	struct stat		file_info;

	switch (client._status)
	{
		case Client::CODE:
			client._res.version = "HTTP/1.1";
			client._res.status_code = BADREQUEST;
			_helper.getErrorPage(client);
			fstat(client._read_fd, &file_info);
			client.setFileToRead(true);
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
			break ;
	}
}