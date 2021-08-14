#include "../includes/Handler.hpp"

Handler::Handler()
{
	
}

Handler::~Handler()
{
	
}

bool			Handler::parseHeaders(std::string &buf, Request &req)
{
	size_t		pos;
	std::string	line;
	std::string	key;
	std::string	value;

	while (!buf.empty())
	{
		ft::getline(buf, line, '\n');
		if (line.size() < 1 || line[0] == '\n' || line[0] == '\r')
			break ;
		if (line.find(':') != std::string::npos)
		{
			pos = line.find(':');
			key = line.substr(0, pos);
			if (line[pos + 1] == ' ')
				value = line.substr(pos + 2);
			else
				value = line.substr(pos + 1);
			if (std::isspace(value[0]) || std::isspace(key[0]) || value.empty() || key.empty())
			{
				req.valid = false;
				return (false);
			}
            req.headers[key] = value;
 			req.headers[key].pop_back(); //remove '\r'
		}
		else
		{
			req.valid = false;
			return (false);
		}
	}
	return (true);
}

bool			Handler::checkSyntax(const Request &req)
{
	std::cout << "1" << std::endl;
	std::cout << "method " << req.method.size() << std::endl;
	std::cout << "uri " << req.uri.size() << std::endl;
	std::cout << "version " << req.version.size() << std::endl;
	if (req.method.size() == 0 || req.uri.size() == 0 || req.version.size() == 0)
		return (false);
			std::cout << "2" << std::endl;

	if (req.method != "GET" && req.method != "POST"
		&& req.method != "HEAD" && req.method != "PUT"
		&& req.method != "CONNECT" && req.method != "TRACE"
		&& req.method != "OPTIONS" && req.method != "DELETE")
		return (false);
			std::cout << "3" << std::endl;

	if (req.method != "OPTIONS" && req.uri[0] != '/') //OPTIONS can have * as uri
		return (false);
			std::cout << "4" << std::endl;

	if (req.version != "HTTP/1.1\r" && req.version != "HTTP/1.1")
		return (false);
			std::cout << "5" << std::endl;

	if (req.headers.find("Host") == req.headers.end())
		return (false);
	return (true);
}

void			Handler::getConf(Client &client, Request &req, std::vector<config> &conf)
{
	std::map<std::string, std::string> elmt;
	std::string		tmp;
	struct stat		info;
	config			to_parse;

	std::cout << "getConf" << std::endl;
	if (!req.valid)
	{
		client._conf["error"] = conf[0]["server|"]["error"];
		return ;
	}
	std::vector<config>::iterator it(conf.begin());
	while (it != conf.end())
	{
		std::cout << req.headers["Host"] << std::endl;
		if (req.headers["Host"] == (*it)["server|"]["server_name"])
		{
			std::cout << "to_parse found!!" << std::endl;
			to_parse = *it;
			break ;
		}
		++it;
	}
	if (it == conf.end())
		to_parse = conf[0];
	std::map<std::string, std::map<std::string, std::string> >::iterator it3(to_parse.begin());
	while (it3 != to_parse.end())
	{
		std::cout << "------------------" << std::endl;
		std::cout << "-" << it3->first << "-" << std::endl;
		std::cout << "------------------" << std::endl;
		it3++;
	}
	std::cout << "to_parse_test : " << to_parse["server| location /|"]["methods"] << std::endl;
	tmp = req.uri;
	std::cout << "tmp : " << tmp << std::endl;
	do
	{
		if (to_parse.find("server| location " + tmp + "|") != to_parse.end())
		{
			std::cout << "config found" << std::endl;
			elmt = to_parse["server| location " + tmp + "|"];
			break ;
		}
		tmp = tmp.substr(0, tmp.find_last_of('/'));
	} while (tmp != "");
	if (elmt.size() == 0)
		if (to_parse.find("server| location /|") != to_parse.end())
			elmt = to_parse["server| location /|"];
	if (elmt.size() > 0)
	{
		std::cout << "elmt size : " << elmt.size() << std::endl;
		client._conf = elmt;
		client._conf["path"] = req.uri.substr(0, req.uri.find("?"));
		if (elmt.find("root") != elmt.end())
			client._conf["path"].replace(0, tmp.size(), elmt["root"]);
	}
	for (std::map<std::string, std::string>::iterator it(to_parse["server|"].begin()); it != to_parse["server|"].end(); ++it)
	{
		if (client._conf.find(it->first) == client._conf.end())
			client._conf[it->first] = it->second;
	}
	lstat(client._conf["path"].c_str(), &info);
	if (S_ISDIR(info.st_mode))
		if (client._conf["index"][0] && client._conf["listing"] != "on")
			client._conf["path"] += "/" + client._conf["index"];
	if (req.method == "GET")
		client._conf["savedpath"] = client._conf["path"];
}

void Handler::parseRequest(Client &client, std::vector<config> &config)
{
    Request request;
	std::string			tmp;
	std::string			buffer;

    buffer = std::string(client._buf);
    ft::getline(buffer, request.method, ' ');
    ft::getline(buffer, request.uri, ' ');
    ft::getline(buffer, request.version, '\n');
    if (parseHeaders(buffer, request))
		request.valid = checkSyntax(request);
	if (request.uri != "*" || request.method != "OPTIONS")
		getConf(client, request, config);
	if (request.valid)
	{
		if (client._conf["root"][0] != '\0')
			chdir(client._conf["root"].c_str());
		if (request.method == "POST" || request.method == "PUT")
			client._status = Client::BODYPARSING;
		else
			client._status = Client::CODE;
	}
	else
	{
		request.method = "BAD";
		client._status = Client::CODE;
	}
	client._req = request;
	std::cout << client._buf << std::endl;
	tmp = client._buf;
	tmp = tmp.substr(tmp.find("\r\n\r\n") + 4);
	strcpy(client._buf, tmp.c_str());
}

void			Handler::parseBody(Client &client)
{
	if (client._req.headers.find("Content-Length") != client._req.headers.end())
		getBody(client);
	else if (client._req.headers["Transfer-Encoding"] == "chunked")
		dechunkBody(client);
	else
	{
		client._req.method = "BAD";
		client._status = Client::CODE;
	}
}

void			Handler::getBody(Client &client)
{
	unsigned int	bytes;

	if (client._chunk.len == 0)
		client._chunk.len = atoi(client._req.headers["Content-Length"].c_str());
	if (client._chunk.len < 0)
	{
		client._req.method = "BAD";
		client._status = Client::CODE;
		return ;
	}
	bytes = strlen(client._buf);
	if (bytes >= client._chunk.len)
	{
		memset(client._buf + client._chunk.len, 0, BUFFER_SIZE - client._chunk.len);
		client._req.body += client._buf;
		client._chunk.len = 0;
		client._status = Client::CODE;
	}
	else
	{
		client._chunk.len -= bytes;
		client._req.body += client._buf;
		memset(client._buf, 0, BUFFER_SIZE + 1);
	}
}

void			Handler::dechunkBody(Client &client)
{
    // while (!client._chunk.done)
    // {
        if (strstr(client._buf, "\r\n") && client._chunk.found == false)
        {
            client._chunk.len = _helper.findLen(client);
            if (client._chunk.len == 0)
                client._chunk.done = true;
            else
                client._chunk.found = true;
        }
        else if (client._chunk.found == true)
            _helper.fillBody(client);
    // }
	if (client._chunk.done)
	{
		memset(client._buf, 0, BUFFER_SIZE + 1);
		client._status = Client::CODE;
		client._chunk.found = false;
		client._chunk.done = false;
		return ;
	}
}

void		Handler::createResponse(Client &client)
{
	std::map<std::string, std::string>::const_iterator b;

	client._response = client._res.version + " " + client._res.status_code + "\r\n";
	b = client._res.headers.begin();
	while (b != client._res.headers.end())
	{
		if (b->second != "")
			client._response += b->first + ": " + b->second + "\r\n";
		++b;
	}
	client._response += "\r\n";
	client._response += client._res.body;
	client._res.clear();
}

void			Handler::createListing(Client &client)
{
	DIR				*dir;
	struct dirent	*cur;

	close(client._read_fd);
	client._read_fd = -1;
	dir = opendir(client._conf["path"].c_str());
	client._res.body = "<html>\n<body>\n";
	client._res.body += "<h1>Directory listing</h1>\n";
	while ((cur = readdir(dir)) != NULL)
	{
		if (cur->d_name[0] != '.')
		{
			client._res.body += "<a href=\"" + client._req.uri;
			if (client._req.uri != "/")
				client._res.body += "/";
			client._res.body += cur->d_name;
			client._res.body += "\">";
			client._res.body += cur->d_name;
			client._res.body += "</a><br>\n";
		}
	}
	closedir(dir);
	client._res.body += "</body>\n</html>\n";
}

void			Handler::negotiate(Client &client)
{
	std::multimap<std::string, std::string> 	languageMap;
	std::multimap<std::string, std::string> 	charsetMap;
	int				fd = -1;
	std::string		path;
	std::string		ext;

	if (client._req.headers.find("Accept-Language") != client._req.headers.end())
		_helper.parseAccept(client, languageMap, "Accept-Language");
	if (client._req.headers.find("Accept-Charset") != client._req.headers.end())
		_helper.parseAccept(client, charsetMap, "Accept-Charset");
	if (!languageMap.empty())
	{
		for (std::multimap<std::string, std::string>::reverse_iterator it(languageMap.rbegin()); it != languageMap.rend(); ++it)
		{
			if (!charsetMap.empty())
			{
				for (std::multimap<std::string, std::string>::reverse_iterator it2(charsetMap.rbegin()); it2 != charsetMap.rend(); ++it2)
				{
					ext = it->second + "." + it2->second;
					path = client._conf["savedpath"] + "." + ext;
					fd = open(path.c_str(), O_RDONLY);
					if (fd != -1)
					{
						client._res.headers["Content-Language"] = it->second;
						break ;
					}
					ext = it2->second + "." + it->second;
					path = client._conf["savedpath"] + "." + ext;
					fd = open(path.c_str(), O_RDONLY);
					if (fd != -1)
					{
						client._res.headers["Content-Language"] = it->second;
						break ;
					}
				}
			}
			else
			{
				ext = it->second;
				path = client._conf["savedpath"] + "." + ext;
				fd = open(path.c_str(), O_RDONLY);
				if (fd != -1)
				{
					client._res.headers["Content-Language"] = it->second;
					break ;
				}
			}
		}
	}
	else if (languageMap.empty())
	{
		if (!charsetMap.empty())
		{
			for (std::multimap<std::string, std::string>::reverse_iterator it2(charsetMap.rbegin()); it2 != charsetMap.rend(); ++it2)
			{
				ext = it2->second;
				path = client._conf["savedpath"] + "." + it2->second;
				fd = open(path.c_str(), O_RDONLY);
				if (fd != -1)
					break ;
			}
		}
	}
	if (fd != -1)
	{
		client._conf["path"] = path;
		client._res.headers["Content-Location"] = client._req.uri + "." + ext;
		if (client._read_fd != -1)
			close(client._read_fd);
		client._read_fd = fd;
		client._res.status_code = OK;
	}
}

void			Handler::execCGI(Client &client)
{
	char			**args = NULL;
	char			**env = NULL;
	std::string		path;
	int				ret;
	int				tubes[2];

	if (client._conf["php"][0] && client._conf["path"].find(".php") != std::string::npos)
		path = client._conf["php"];
	else if (client._conf["exec"][0])
		path = client._conf["exec"];
	else
		path = client._conf["path"];
	close(client._read_fd);
	client._read_fd = -1;	
	args = (char **)(malloc(sizeof(char *) * 3));
	args[0] = strdup(path.c_str());
	args[1] = strdup(client._conf["path"].c_str());
	args[2] = NULL;
	env = _helper.setEnv(client);
	client._tmp_fd = open(TMP_PATH, O_WRONLY | O_CREAT, 0666);
	pipe(tubes);
	if ((client._cgi_pid = fork()) == 0)
	{
		close(tubes[1]);
		dup2(tubes[0], 0);
		dup2(client._tmp_fd, 1);
		errno = 0;
		ret = execve(path.c_str(), args, env);
		if (ret == -1)
		{
			std::cerr << "Error with CGI: " << strerror(errno) << std::endl;
			exit(1);
		}
	}
	else
	{
		close(tubes[0]);
		client._write_fd = tubes[1];
		client._read_fd = open(TMP_PATH, O_RDONLY);
		client.setFileToWrite(true);
	}
	ft::freeAll(args, env);
}

void		Handler::parseCGIResult(Client &client)
{
	size_t			pos;
	std::string		headers;
	std::string		key;
	std::string		value;

	if (client._res.body.find("\r\n\r\n") == std::string::npos)
		return ;
	headers = client._res.body.substr(0, client._res.body.find("\r\n\r\n") + 1);
	pos = headers.find("Status");
	if (pos != std::string::npos)
	{
		client._res.status_code.clear();
		pos += 8;
		while (headers[pos] != '\r')
		{
			client._res.status_code += headers[pos];
			pos++;
		}
	}
	pos = 0;
	while (headers[pos])
	{
		while (headers[pos] && headers[pos] != ':')
		{
			key += headers[pos];
			++pos;
		}
		++pos;
		while (headers[pos] && headers[pos] != '\r')
		{
			value += headers[pos];
			++pos;
		}
		client._res.headers[key] = value;
		key.clear();
		value.clear();
		if (headers[pos] == '\r')
			pos++;
		if (headers[pos] == '\n')
			pos++;
	}
	pos = client._res.body.find("\r\n\r\n") + 4;
	client._res.body = client._res.body.substr(pos);
	client._res.headers["Content-Length"] = std::to_string(client._res.body.size());
}
