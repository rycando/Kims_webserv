#include "Server.hpp"

int		Server::readRequest(std::vector<Client*>::iterator it)
{
	int 		bytes;
	int			ret;
	Client		*client = NULL;

	client = *it;
	bytes = strlen(client->rBuf);
	ret = read(client->fd, client->rBuf + bytes, BUFFER_SIZE - bytes);
	bytes += ret;
	if (ret > 0)
	{
		client->rBuf[bytes] = '\0';
		if (strstr(client->rBuf, "\r\n\r\n") != NULL
			&& client->status != Client::BODYPARSING)
		{
			client->last_date = ft::getDate();
			_handler.parseRequest(*client, _conf);
			client->setWriteState(true);
		}
		if (client->status == Client::BODYPARSING)
			_handler.parseBody(*client);
		return (1);
	}
	else
	{
		*it = NULL;
		_clients.erase(it);
		if (client)
			delete client;
		return (0);
	}
}

Server::Server(/* args */)
{
}

Server::~Server()
{
}
