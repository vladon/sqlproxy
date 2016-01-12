#include <cstdint>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "proxy_server.h"

int main(int argc, char ** argv)
{
    namespace po = boost::program_options;

    proxy_server_config_t proxy_server_config{};

    po::options_description desc("Options");
    desc.add_options()
        ("bind-host,h", po::value<host_t>(&proxy_server_config.bind_host)->default_value("0.0.0.0"), "bind host")
        ("bind-port,p", po::value<port_t>(&proxy_server_config.bind_port)->default_value(3306), "bind port")
        ("remote-host,r", po::value<host_t>(&proxy_server_config.remote_host)->required(), "remote host")
        ("remote-port,s", po::value<port_t>(&proxy_server_config.remote_port)->default_value(3306), "remote port")
        ("help,h", "this help message");

    po::variables_map opts;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), opts);

    if (opts.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    try
    {
        po::notify(opts);
    }
    catch (const std::exception & ex)
    {
        std::cerr << "Error parsing command-line: " << ex.what() << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }

    proxy_server my_proxy{ proxy_server_config };
    my_proxy.run();

    return 0;
}
