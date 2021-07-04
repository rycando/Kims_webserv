#include "Config.hpp"


std::string Config::read_file(char *file) {
    int fd;
    int ret;
    char buf[4096];
    std::string str;

    fd = open(file, O_RDONLY);
    while ((ret = read(fd, buf, 4095)) > 0)
    {
        buf[ret] = '\0';
        str += buf;
    }
    close(fd);
    
    return str;
}

std::pair<std::string, std::string> Config::parse_elmt(std::string line)
{
    size_t pos;
    std::string key;
    std::string value;

    while(std::isspace(line[0]))
        line.erase(line.begin());
    pos = 0;
    while (line[pos] && !std::isspace(line[pos]))
		key += line[pos++];
    while (std::isspace(line[pos]))
        pos++;
    while (line[pos] && line[pos] != ';' && line[pos] != '{')
        value += line[pos++];
    while(std::isspace(line[0]))
        line.erase(line.begin());
    while(std::isspace(value[value.size()-1]))
        line.erase(line.end() - 1);
    std::pair<std::string, std::string> tmp(key, value);
    key.clear();
	value.clear();
    return tmp;
}

void Config::parse_conf(std::string &buf, std::string line, std::string &context, config &config)
{
    size_t pos;
    std::string tmp;
    std::pair<std::string, std::string> value;

        while (line.empty())
        {
            ft::getline(buf, line, '\n');
            while(std::isspace(line[0]))
                line.erase(line.begin());
        }
        if ((pos = line.find('{')) == std::string::npos)
        {
            std::cout << line << std::endl;
            throw Config::InvalidConfigFileException(3);
        }
        tmp = std::string(line, 0, pos);
        while (std::isspace(tmp[tmp.size() - 1]))
            tmp.erase(tmp.end() - 1);
        if (context.empty() && tmp.compare(0, 6, "server"))
            throw Config::InvalidConfigFileException(4);
        context += tmp + "|";
        ft::getline(buf, line, '\n');;
        while ((pos = line.find('}')) == std::string::npos)
        {
            if ((pos = line.find(';')) != std::string::npos)
            {
                value = parse_elmt(line);
                config[context].insert(value);
            }
            else if (line.find('{') != std::string::npos)
                parse_conf(buf, line, context, config);
            ft::getline(buf, line, '\n');;
        }
        while(std::isspace(line[0]))
            line.erase(line.begin());
        if (line[0] != '}')
        {
            std::cout << line << std::endl;
            throw Config::InvalidConfigFileException(5);
        }
        while (std::isspace(line[1]))
            line.erase(line.begin() + 1);
        if (line[1])
            throw Config::InvalidConfigFileException(6);
        context.pop_back();
		context = context.substr(0, context.find_last_of('|') + 1);
        line.clear();

}

void Config::parse(char *file, std::vector<Server> &servers) {
    std::string buf;
    std::string context;
    Config::config config;
    std::string line;
    Server server;


    buf = read_file(file);
    while (!buf.empty())
    {
        parse_conf(buf, line, context, config);
        std::vector<Server>::iterator it(servers.begin());
        while (it != servers.end())
        {
            if (config["server|"]["listen"] == it->_conf.back()["server|"]["listen"])
            {
                std::cout << config["server|"]["listen"] << " " << it->_conf.back()["server|"]["listen"] << std::endl;
                std::vector<Config::config>::iterator it2(it->_conf.begin());
                while (it2 != it->_conf.end())
                {
                    if (config["server|"]["server_name"] == (*it2)["server|"]["server_name"])
                    {
                        throw(Config::InvalidConfigFileException(7));
                    }
                    it2++;
                }
                it->_conf.push_back(config);
                break ;
            }
            ++it;
        }
        if (it == servers.end())
        {
            server._conf.push_back(config);
            servers.push_back(server);
        }
        config.clear();
    }
}


Config::InvalidConfigFileException::InvalidConfigFileException(void) {this->line = 0;}

Config::InvalidConfigFileException::InvalidConfigFileException(size_t d) {
	this->line = d;
	this->error = "line " + std::to_string(this->line) + ": Invalid Config File";
}

Config::InvalidConfigFileException::~InvalidConfigFileException(void) throw() {}

size_t						Config::InvalidConfigFileException::getLine(void) const
{
	return (this->line);
}

const char					*Config::InvalidConfigFileException::what(void) const throw()
{
	if (this->line)
		return (error.c_str());
	return ("Invalid Config File");
}


Config::Config(/* args */)
{
}

Config::~Config()
{
}