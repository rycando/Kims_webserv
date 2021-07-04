#include "Config.hpp"
#include <iostream>

std::vector<Server>		g_servers;

int main(int argc, char **argv) 
{
    Config config;

    if (argc != 2)
        return 1;

    try
    {
        config.parse(argv[1], g_servers);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (1);
    }
}