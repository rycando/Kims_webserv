#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Webserv.hpp"
#include "Server.hpp"

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
	    
	    Config();
	    ~Config();

	    void parse(char *file, std::vector<Server> &servers);
		int  getMaxFd(std::vector<Server> &servers);

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
