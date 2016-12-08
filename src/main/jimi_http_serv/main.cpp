
#include "boost_asio_msvc.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <exception>
#include <boost/program_options.hpp>

#include "utils.hpp"

void make_spaces(std::string & spaces, std::size_t size)
{
    spaces = "";
    for (std::size_t i = 0; i < size; ++i)
        spaces += " ";
}

void print_usage(const std::string & app_name, const boost::program_options::options_description & options_desc)
{
    std::string leader_spaces;
    make_spaces(leader_spaces, app_name.size());

    std::cerr << std::endl;
    std::cerr << options_desc << std::endl;

    std::cerr << "Usage: " << std::endl << std::endl
              << "  " << app_name.c_str()      << " --host=<host> --port=<port> --mode=<mode> --test=<test>" << std::endl
              << "  " << leader_spaces.c_str() << " [--pipeline=1] [--packet_size=64] [--thread-num=0]" << std::endl
              << std::endl
              << "For example: " << std::endl << std::endl
              << "  " << app_name.c_str()      << " --host=127.0.0.1 --port=9000 --mode=echo --test=pingpong" << std::endl
              << "  " << leader_spaces.c_str() << " --pipeline=10 --packet-size=64 --thread-num=8" << std::endl
              << std::endl
              << "  " << app_name.c_str() << " -s 127.0.0.1 -p 9000 -m echo -t pingpong -l 10 -k 64 -n 8" << std::endl;
    std::cerr << std::endl;
}

int main(int argc, char * argv[])
{
    std::string app_name;
    std::string test_mode, test_method, nodelay, rpc_topic;
    std::string server_ip, server_port;
    std::string mode, test, cmd, cmd_value;
    int32_t pipeline = 1, packet_size = 0, thread_num = 0, need_echo = 1;

    namespace options = boost::program_options;
    options::options_description desc("Command list");
    desc.add_options()
        ("help,h",                                                                                  "usage info")
        ("host,s",          options::value<std::string>(&server_ip)->default_value("127.0.0.1"),    "server host or ip address")
        ("port,p",          options::value<std::string>(&server_port)->default_value("9000"),       "server port")
        ("mode,m",          options::value<std::string>(&test_mode)->default_value("echo"),         "test mode = [echo]")
        ("test,t",          options::value<std::string>(&test_method)->default_value("pingpong"),   "test method = [pingpong, qps, latency, throughput]")
        ("pipeline,l",      options::value<int32_t>(&pipeline)->default_value(1),                   "pipeline numbers")
        ("packet-size,k",   options::value<int32_t>(&packet_size)->default_value(64),               "packet size")
        ("thread-num,n",    options::value<int32_t>(&thread_num)->default_value(0),                 "thread numbers")
        ("nodelay,y",       options::value<std::string>(&nodelay)->default_value("false"),          "TCP socket nodelay = [0 or 1, true or false]")
        ("echo,e",          options::value<int32_t>(&need_echo)->default_value(1),                  "whether the server need echo")
        ;

    // parse command line
    options::variables_map args_map;
    try {
        options::store(options::parse_command_line(argc, argv, desc), args_map);
    }
    catch (const std::exception & ex) {
        std::cout << "Exception is: " << ex.what() << std::endl;
    }
    options::notify(args_map);

    app_name = get_app_name(argv[0]);

    // help
    if (args_map.count("help") > 0) {
        std::string app_name = get_app_name(argv[0]);
        print_usage(app_name, desc);
        exit(EXIT_FAILURE);
    }

    return 0;
}
