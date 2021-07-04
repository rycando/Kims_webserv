#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>

class Server
{
friend class Config;
typedef std::map<std::string, std::string> elmt;
typedef std::map<std::string, elmt> config;

private:
    std::vector<config> _conf;

public:
    Server(/* args */);
    ~Server();
};



#endif