#include "utils.hpp"
#include <fstream>

namespace ft
{
	void	getline(std::string &buffer, std::string &line, char delim)
	{
		size_t					pos;

		pos = buffer.find(delim);
		if (pos != std::string::npos)
		{
			line = std::string (buffer, 0, pos++);
			buffer = buffer.substr(pos);
		}
		else
		{
			if (buffer[buffer.size() - 1] == delim)
				buffer = buffer.substr(buffer.size());
			else
			{
				line = buffer;
				buffer = buffer.substr(buffer.size());
			}
		}
	}

    int		getpower(int nb, int power)
	{
		if (power < 0)
			return (0);
		if (power == 0)
			return (1);
		return (nb * getpower(nb, power - 1));
	}

	std::string		getDate()
	{
		struct timeval	time;
		struct tm		*tm;
		char			buf[BUFFER_SIZE + 1];
		int				ret;

		gettimeofday(&time, NULL);
		tm = localtime(&time.tv_sec);
		ret = strftime(buf, BUFFER_SIZE, "%a, %d %b %Y %T %Z", tm);
		buf[ret] = '\0';
		return (buf);
	}

	void	freeAll(char **args, char **env)
	{
		free(args[0]);
		free(args[1]);
		free(args);
		int i = 0;
		while (env[i])
		{
			free(env[i]);
			++i;
		}
		free(env);
	}

	void				logger(std::string const &message, int param)
	{
		std::ofstream	writeFile;

		if (param > 0)
			std::cout << message << std::endl;
		else
		{
			writeFile.open("./WebLog", std::ofstream::app);
			writeFile << message << std::endl;
			writeFile.close();
		}
	}
}