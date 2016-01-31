#include "stdafx.h"

#include <cstdint>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "basic_proxy_session.h"
#include "basic_proxy_server.h"

int main(int argc, char ** argv)
{
    namespace po = boost::program_options;
    using namespace sql_proxy;

    proxy_server_config_t config{};

    po::options_description desc("Options");
    desc.add_options()
        ("bind-host,h", po::value<host_t>(&config.local_ip_addr)->default_value("0.0.0.0"), "bind IP")
        ("bind-port,p", po::value<port_t>(&config.local_port)->default_value(3306), "bind port")
        ("remote-host,r", po::value<host_t>(&config.upstream_ip_addr)->required(), "remote IP")
        ("remote-port,s", po::value<port_t>(&config.upstream_port)->default_value(3306), "remote port")
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

    boost::asio::io_service ios;

    try
    {
        std::cout << "Starting sql_proxy: "
            << config.local_ip_addr << ":" << config.local_port
            << " -> "
            << config.upstream_ip_addr << ":" << config.upstream_port
            << "\n\nPress Ctrl+Break to abort"
            << std::endl;

        basic_proxy_server my_proxy{ ios, config };
        my_proxy.accept_connections();

        ios.run();
    }
    catch (const std::exception & ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
