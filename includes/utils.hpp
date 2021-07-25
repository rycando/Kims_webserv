#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include <iostream>
#include <sys/time.h>

#define BUFFER_SIZE 32768

namespace ft
{
	void		getline(std::string &buffer, std::string &line, char delim);
	int			getpower(int nb, int power);
	std::string	getDate();

}

#endif
