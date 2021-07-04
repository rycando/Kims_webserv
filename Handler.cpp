#include "Handler.hpp"

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
	if (req.method.size() == 0 || req.uri.size() == 0
		|| req.version.size() == 0)
		return (false);
	if (req.method != "GET" && req.method != "POST"
		&& req.method != "HEAD" && req.method != "PUT"
		&& req.method != "CONNECT" && req.method != "TRACE"
		&& req.method != "OPTIONS" && req.method != "DELETE")
		return (false);
	if (req.method != "OPTIONS" && req.uri[0] != '/') //OPTIONS can have * as uri
		return (false);
	if (req.version != "HTTP/1.1\r" && req.version != "HTTP/1.1")
		return (false);
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

	if (!req.valid)
	{
		client.conf["error"] = conf[0]["server|"]["error"];
		return ;
	}
	std::vector<config>::iterator it(conf.begin());
	while (it != conf.end())
	{
		if (req.headers["Host"] == (*it)["server|"]["server_name"])
		{
			to_parse = *it;
			break ;
		}
		++it;
	}
	if (it == conf.end())
		to_parse = conf[0];
	tmp = req.uri;
	do
	{
		if (to_parse.find("server|location " + tmp + "|") != to_parse.end())
		{
			elmt = to_parse["server|location " + tmp + "|"];
			break ;
		}
		tmp = tmp.substr(0, tmp.find_last_of('/'));
	} while (tmp != "");
	if (elmt.size() == 0)
		if (to_parse.find("server|location /|") != to_parse.end())
			elmt = to_parse["server|location /|"];
	if (elmt.size() > 0)
	{
		client.conf = elmt;
		client.conf["path"] = req.uri.substr(0, req.uri.find("?"));
		if (elmt.find("root") != elmt.end())
			client.conf["path"].replace(0, tmp.size(), elmt["root"]);
	}
	for (std::map<std::string, std::string>::iterator it(to_parse["server|"].begin()); it != to_parse["server|"].end(); ++it)
	{
		if (client.conf.find(it->first) == client.conf.end())
			client.conf[it->first] = it->second;
	}
	lstat(client.conf["path"].c_str(), &info);
	if (S_ISDIR(info.st_mode))
		if (client.conf["index"][0] && client.conf["listing"] != "on")
			client.conf["path"] += "/" + client.conf["index"];
	if (req.method == "GET")
		client.conf["savedpath"] = client.conf["path"];
}

void Handler::parseRequest(Client &client, std::vector<config> &config)
{
    Request request;
	std::string			tmp;
	std::string			buffer;

    buffer = std::string(client.rBuf);
    ft::getline(buffer, request.method, ' ');
    ft::getline(buffer, request.uri, ' ');
    ft::getline(buffer, request.version, '\n');
    if (parseHeaders(buffer, request))
		request.valid = checkSyntax(request);
	if (request.uri != "*" || request.method != "OPTIONS")
		getConf(client, request, config);
	if (request.valid)
	{
		if (client.conf["root"][0] != '\0')
			chdir(client.conf["root"].c_str());
		if (request.method == "POST" || request.method == "PUT")
			client.status = Client::BODYPARSING;
		else
			client.status = Client::CODE;
	}
	else
	{
		request.method = "BAD";
		client.status = Client::CODE;
	}
	client.req = request;
	tmp = client.rBuf;
	tmp = tmp.substr(tmp.find("\r\n\r\n") + 4);
	strcpy(client.rBuf, tmp.c_str());
}

void			Handler::parseBody(Client &client)
{
	if (client.req.headers.find("Content-Length") != client.req.headers.end())
		getBody(client);
	else if (client.req.headers["Transfer-Encoding"] == "chunked")
		dechunkBody(client);
	else
	{
		client.req.method = "BAD";
		client.status = Client::CODE;
	}
}

void			Handler::getBody(Client &client)
{
	unsigned int	bytes;

	if (client.chunk.len == 0)
		client.chunk.len = atoi(client.req.headers["Content-Length"].c_str());
	if (client.chunk.len < 0)
	{
		client.req.method = "BAD";
		client.status = Client::CODE;
		return ;
	}
	bytes = strlen(client.rBuf);
	if (bytes >= client.chunk.len)
	{
		memset(client.rBuf + client.chunk.len, 0, BUFFER_SIZE - client.chunk.len);
		client.req.body += client.rBuf;
		client.chunk.len = 0;
		client.status = Client::CODE;
	}
	else
	{
		client.chunk.len -= bytes;
		client.req.body += client.rBuf;
		memset(client.rBuf, 0, BUFFER_SIZE + 1);
	}
}

void			Handler::dechunkBody(Client &client)
{
    // while (!client.chunk.done)
    // {
        if (strstr(client.rBuf, "\r\n") && client.chunk.found == false)
        {
            client.chunk.len = _helper.findLen(client);
            if (client.chunk.len == 0)
                client.chunk.done = true;
            else
                client.chunk.found = true;
        }
        else if (client.chunk.found == true)
            _helper.fillBody(client);
    // }
	if (client.chunk.done)
	{
		memset(client.rBuf, 0, BUFFER_SIZE + 1);
		client.status = Client::CODE;
		client.chunk.found = false;
		client.chunk.done = false;
		return ;
	}
}