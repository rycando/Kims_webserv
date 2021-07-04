#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Server.hpp"
#include <map>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cctype>
#include <map>
#include "utils.h"

class Config
{

    typedef std::map<std::string, std::string> 	elmt;
	typedef std::map<std::string, elmt>			config;    
private:

    std::pair<std::string, std::string> parse_elmt(std::string line);
    std::string read_file(char *file);
    void getline(std::string &buf, std::string &line);
    void parse_conf(std::string &buf, std::string line, std::string &context, config &config);




public:
    
    Config(/* args */);
    ~Config();

    void parse(char *file, std::vector<Server> &servers);


    class InvalidConfigFileException: public std::exception
	{
		private:
			size_t						line;
			std::string					error;

			InvalidConfigFileException(void);

		public:
			InvalidConfigFileException(size_t d);
			virtual ~InvalidConfigFileException(void) throw();
			size_t						getLine(void) const; 
			virtual const char			*what(void) const throw();
	};
};



#endif