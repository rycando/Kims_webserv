#include "../includes/Helper.hpp"

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

Helper::Helper()
{
	assignMIME();
}

Helper::~Helper()
{
	
}

std::string		Helper::decode64(const char *data)
{
	while (*data != ' ')
		data++;
	data++;
	unsigned int len = strlen(data);
	unsigned char* p = (unsigned char*)data;
    int pad = len > 0 && (len % 4 || p[len - 1] == '=');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    std::string str(L / 4 * 3 + pad, '\0');

    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad)
    {
        int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
        str[str.size() - 1] = n >> 16;

        if (len > L + 2 && p[L + 2] != '=')
        {
            n |= B64index[p[L + 2]] << 6;
            str.push_back(n >> 8 & 0xFF);
        }
    }
    if (str.back() == 0)
    	str.pop_back();
    return (str);
}

void			Helper::getErrorPage(Client &client)
{
	std::string		path;

	path = client._conf["error"] + "/" + client._res.status_code.substr(0, 3) + ".html";
	client._conf["path"] = path;
	client._read_fd = open(path.c_str(), O_RDONLY);
}

int				Helper::fromHexa(const char *nb)
{
	char	base[17] = "0123456789abcdef";
	char	base2[17] = "0123456789ABCDEF";
	int		result = 0;
	int		i;
	int		index;

	i = -1;
	while (nb[++i])
	{
		int j = -1;
		while (base[++j])
			if (nb[i] == base[j])
			{
				index = j;
				break ;
			}
		if (j == 16)
		{
			j = -1;
			while (base2[++j])
				if (nb[i] == base2[j])
				{
					index = j;
					break ;
				}
		}
		result += index * ft::getpower(16, (strlen(nb) - 1) - i);
	}
	return (result);
}

int				Helper::findLen(Client &client)
{
	std::string		to_convert;
	int				len;
	std::string		tmp;

	to_convert = client._buf;
	to_convert = to_convert.substr(0, to_convert.find("\r\n"));
	while (to_convert[0] == '\n')
		to_convert.erase(to_convert.begin());
	if (to_convert.size() == 0)
		len = 0;
	else
		len = fromHexa(to_convert.c_str());
	tmp = client._buf;
	tmp = tmp.substr(tmp.find("\r\n") + 2);
	strcpy(client._buf, tmp.c_str());
	return (len);
}

char			**Helper::setEnv(Client &client)
{
	char											**env;
	std::map<std::string, std::string> 				envMap;
	size_t											pos;

	envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	envMap["SERVER_SOFTWARE"] = "webserv";
	envMap["REQUEST_URI"] = client._req.uri;
	envMap["REQUEST_METHOD"] = client._req.method;
	envMap["REMOTE_ADDR"] = client._ip;
	envMap["PATH_INFO"] = client._req.uri;
	envMap["PATH_TRANSLATED"] = client._conf["path"];
	envMap["CONTENT_LENGTH"] = std::to_string(client._req.body.size());

	if (client._req.uri.find('?') != std::string::npos)
		envMap["QUERY_STRING"] = client._req.uri.substr(client._req.uri.find('?') + 1);
	else
		envMap["QUERY_STRING"];
	if (client._req.headers.find("Content-Type") != client._req.headers.end())
		envMap["CONTENT_TYPE"] = client._req.headers["Content-Type"];
	if (client._conf.find("exec") != client._conf.end())
		envMap["SCRIPT_NAME"] = client._conf["exec"];
	else
		envMap["SCRIPT_NAME"] = client._conf["path"];
	if (client._conf["listen"].find(":") != std::string::npos)
	{
		envMap["SERVER_NAME"] = client._conf["listen"].substr(0, client._conf["listen"].find(":"));
		envMap["SERVER_PORT"] = client._conf["listen"].substr(client._conf["listen"].find(":") + 1);
	}
	else
		envMap["SERVER_PORT"] = client._conf["listen"];
	if (client._req.headers.find("Authorization") != client._req.headers.end())
	{
		pos = client._req.headers["Authorization"].find(" ");
		envMap["AUTH_TYPE"] = client._req.headers["Authorization"].substr(0, pos);
		envMap["REMOTE_USER"] = client._req.headers["Authorization"].substr(pos + 1);
		envMap["REMOTE_IDENT"] = client._req.headers["Authorization"].substr(pos + 1);
	}
	if (client._conf.find("php") != client._conf.end() && client._req.uri.find(".php") != std::string::npos)
		envMap["REDIRECT_STATUS"] = "200";

	std::map<std::string, std::string>::iterator b = client._req.headers.begin();
	while (b != client._req.headers.end())
	{
		envMap["HTTP_" + b->first] = b->second;
		++b;
	}
	env = (char **)malloc(sizeof(char *) * (envMap.size() + 1));
	std::map<std::string, std::string>::iterator it = envMap.begin();
	int i = 0;
	while (it != envMap.end())
	{
		env[i] = strdup((it->first + "=" + it->second).c_str());
		++i;
		++it;
	}
	env[i] = NULL;
	return (env);
}

void			Helper::fillBody(Client &client)
{
	std::string		tmp;

	tmp = client._buf;
	if (tmp.size() > client._chunk.len)
	{
		client._req.body += tmp.substr(0, client._chunk.len);

		std::cout << "===================================\n";
		std::cout << client._req.body << std::endl;
		std::cout << "===================================\n";


		tmp = tmp.substr(client._chunk.len + 1);
		memset(client._buf, 0, BUFFER_SIZE + 1);
		strcpy(client._buf, tmp.c_str());
		client._chunk.len = 0;
		client._chunk.found = false;
	}
	else
	{
		client._req.body += tmp;

		std::cout << "===================================\n";
		std::cout << client._req.body << std::endl;
		std::cout << "===================================\n";
		
		client._chunk.len -= tmp.size();
        client._chunk.done = true;
		memset(client._buf, 0, BUFFER_SIZE + 1);
	}
}

void			Helper::parseAccept(Client &client, std::multimap<std::string, std::string> &map, std::string header)
{
	std::string							charset;
	std::string							to_parse;
	std::string							q;

	to_parse = client._req.headers[header];
	int i = 0;
	while (to_parse[i] != '\0')
	{
		charset.clear();
		q.clear();
		while (to_parse[i] && to_parse[i] != ',' && to_parse[i] != ';')
		{
			charset += to_parse[i];
			++i;
		}
		if (to_parse[i] == ',' || to_parse[i] == '\0')
			q = "1";
		else if (to_parse[i] == ';')
		{
			i += 3;
			while (to_parse[i] && to_parse[i] != ',')
			{
				q += to_parse[i];
				++i;
			}
		}
		if (to_parse[i])
			++i;
		std::pair<std::string, std::string>	pair(q, charset);
		map.insert(pair);
	}
}

std::string		Helper::getLastModified(std::string path)
{
	char		buf[BUFFER_SIZE + 1];
	int			ret;
	struct tm	*tm;
	struct stat	file_info;

	if (lstat(path.c_str(), &file_info) == -1)
		return ("");
	tm = localtime(&file_info.st_mtime);
	ret = strftime(buf, BUFFER_SIZE, "%a, %d %b %Y %T %Z", tm);
	buf[ret] = '\0';
	return (buf);
}

std::string		Helper::findType(Client &client)
{
	std::string 	extension;
	size_t			pos;

	if (client._conf["path"].find_last_of('.') != std::string::npos)
	{
		pos = client._conf["path"].find('.');
		extension = client._conf["path"].substr(pos, client._conf["path"].find('.', pos + 1) - pos);
		if (MIMETypes.find(extension) != MIMETypes.end())
			return (MIMETypes[extension]);
		else
			return (MIMETypes[".bin"]);
	}
	return ("");
}

void			Helper::assignMIME()
{
	MIMETypes[".txt"] = "text/plain";
	MIMETypes[".bin"] = "application/octet-stream";
	MIMETypes[".jpeg"] = "image/jpeg";
	MIMETypes[".jpg"] = "image/jpeg";
	MIMETypes[".html"] = "text/html";
	MIMETypes[".htm"] = "text/html";
	MIMETypes[".png"] = "image/png";
	MIMETypes[".bmp"] = "image/bmp";
	MIMETypes[".pdf"] = "application/pdf";
	MIMETypes[".tar"] = "application/x-tar";
	MIMETypes[".json"] = "application/json";
	MIMETypes[".css"] = "text/css";
	MIMETypes[".js"] = "application/javascript";
	MIMETypes[".mp3"] = "audio/mpeg";
	MIMETypes[".avi"] = "video/x-msvideo";
}
