#include "Helper.hpp"

int				Helper::fromHexa(const char *nb)
{
	char	base[17] = "0123456789abcdef";
	char	base2[17] = "0123456789ABCDEF";
	int		result = 0;
	int		i;
	int		index;

	i = 0;
	while (nb[i])
	{
		int j = 0;
		while (base[j])
		{
			if (nb[i] == base[j])
			{
				index = j;
				break ;
			}
			j++;
		}
		if (j == 16)
		{
			j = 0;
			while (base2[j])
			{
				if (nb[i] == base2[j])
				{
					index = j;
					break ;
				}
				j++;
			}
		}
		result += index * ft::getpower(16, (strlen(nb) - 1) - i);
		i++;
	}
	return (result);
}

int				Helper::findLen(Client &client)
{
	std::string		to_convert;
	int				len;
	std::string		tmp;

	to_convert = client.rBuf;
	to_convert = to_convert.substr(0, to_convert.find("\r\n"));
	while (to_convert[0] == '\n')
		to_convert.erase(to_convert.begin());
	if (to_convert.size() == 0)
		len = 0;
	else
		len = fromHexa(to_convert.c_str());
	tmp = client.rBuf;
	tmp = tmp.substr(tmp.find("\r\n") + 2);
	strcpy(client.rBuf, tmp.c_str());
	return (len);
}

void			Helper::fillBody(Client &client)
{
	std::string		tmp;

	tmp = client.rBuf;
	if (tmp.size() > client.chunk.len)
	{
		client.req.body += tmp.substr(0, client.chunk.len);
		tmp = tmp.substr(client.chunk.len + 1);
		memset(client.rBuf, 0, BUFFER_SIZE + 1);
		strcpy(client.rBuf, tmp.c_str());
		client.chunk.len = 0;
		client.chunk.found = false;
	}
	else
	{
		client.req.body += tmp;
		client.chunk.len -= tmp.size();
        client.chunk.done = true;
		memset(client.rBuf, 0, BUFFER_SIZE + 1);
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