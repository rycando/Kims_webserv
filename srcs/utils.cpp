#include "utils.hpp"
#include <fstream>
#include <time.h>

static std::string 		old_message = "";
static std::vector<int>	p_list;

namespace ft
{

	void				getline(std::string &buffer, std::string &line, char delim)
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

    int					getpower(int nb, int power)
	{
		if (power < 0)
			return (0);
		if (power == 0)
			return (1);
		return (nb * getpower(nb, power - 1));
	}

	std::string			getDate()
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

	void				freeAll(char **args, char **env)
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
		{
			if (param == 1 && old_message != message)
			{
				std::cout << "[" << return_current_time_and_date() << "] " << message << std::endl;
				old_message = message;
			}
			else if (message.find("answered") != std::string::npos)
			{
				std::vector<int>::iterator it = find(p_list.begin(), p_list.end(), param);
				if (it == p_list.end())
				{
					std::cout << "[" << return_current_time_and_date() << "] " << message << std::endl;
					p_list.push_back(param);
				}
			}
		}
		else
		{
			writeFile.open("/client_status.log", std::ios::out | std::ios::app);
			writeFile << "[" << return_current_time_and_date() << "] " << message << std::endl;
			writeFile.close();
		}
	}

	std::string			return_current_time_and_date()
	{
	    time_t now = time(0);
	    struct tm tstruct;
	    char buf[80];

	    tstruct = *localtime(&now);
	    strftime(buf, sizeof(buf), "%X", &tstruct);
	    return (buf);
	}
}