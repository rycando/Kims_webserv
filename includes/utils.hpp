#ifndef UTILS_HPP
# define UTILS_HPP

#include "Webserv.hpp"
#include <vector>
#include <time.h>

#define BUFFER_SIZE 32768

typedef std::map<std::string, std::string> 	elmt;
typedef std::map<std::string, elmt>			config;

namespace ft
{
	void				getline(std::string &buffer, std::string &line, char delim);
	int					getpower(int nb, int power);
	std::string			getDate();
	void				freeAll(char **args, char **env);
	void				logger(std::string const &message, int param);
	std::string			return_current_time_and_date();
}

void	show_elmt(std::map<std::string, std::string> &map);
void    show_config(std::map<std::string, elmt> &map);

#endif
