
#include "boost_asio_msvc.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <exception>
#include <boost/program_options.hpp>

#include "utils.hpp"
#include "atomic_padding.hpp"

#define DEFAULT_PACKET_SIZE 32
#define MAX_PACKET_SIZE     (128 * 1024)

enum http_server_mode_t {
    mode_http_server,
    mode_echo_server,
};

std::string g_server_ip;
std::string g_server_port;

uint32_t g_mode         = http_server_mode_t::mode_http_server;
uint32_t g_nodelay      = 0;
uint32_t g_need_echo    = 1;
uint32_t g_packet_size  = 64;

std::string g_mode_str      = "echo";
std::string g_nodelay_str   = "false";

namespace jimi {

extern atomic_padding<uint64_t> g_query_count;
extern atomic_padding<uint32_t> g_client_count;

extern atomic_padding<uint64_t> g_recv_bytes;
extern atomic_padding<uint64_t> g_send_bytes;

} // namespace jimi

jimi::atomic_padding<uint64_t> jimi::g_query_count(0);
jimi::atomic_padding<uint32_t> jimi::g_client_count(0);

jimi::atomic_padding<uint64_t> jimi::g_recv_bytes(0);
jimi::atomic_padding<uint64_t> jimi::g_send_bytes(0);

void run_http_server(const std::string & host, const std::string & port,
                     uint32_t packet_size, uint32_t thread_num,
                     bool confirm = false)
{
    //
}

void run_http_server_ex(const std::string & host, const std::string & port,
                        uint32_t packet_size, uint32_t thread_num,
                        bool confirm = false)
{
    //
}

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
    std::string server_ip, server_port;
    std::string test_mode, test_method, nodelay, rpc_topic;
    std::string mode_str, test_str, cmd, cmd_value;
    int32_t mode = 0;
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

    // host
    if (args_map.count("host") > 0) {
        server_ip = args_map["host"].as<std::string>();
    }
    std::cout << "host: " << server_ip.c_str() << std::endl;
    if (!is_valid_ip_v4(server_ip)) {
        std::cerr << "Error: ip address \"" << server_ip.c_str() << "\" format is wrong." << std::endl;
        exit(EXIT_FAILURE);
    }

    // port
    if (args_map.count("port") > 0) {
        server_port = args_map["port"].as<std::string>();
    }
    std::cout << "port: " << server_port.c_str() << std::endl;
    if (!is_socket_port(server_port)) {
        std::cerr << "Error: port [" << server_port.c_str() << "] number must be range in (0, 65535]." << std::endl;
        exit(EXIT_FAILURE);
    }

    // mode
    if (args_map.count("mode") > 0) {
        mode_str = args_map["mode"].as<std::string>();
    }

    // Get the mode info
    if (mode_str == "http") {
        g_mode = mode_http_server;
        g_mode_str = "Http Server";
    }
    if (mode_str == "echo") {
        g_mode = mode_echo_server;
        g_mode_str = "Http Echo Server";
    }
    else {
        g_mode = mode_http_server;
        g_mode_str = "Http Server";
    }
    std::cout << "mode string: " << mode_str.c_str() << std::endl;
    std::cout << "mode       : " << g_mode_str.c_str() << std::endl;

    // packet-size
    if (args_map.count("packet-size") > 0) {
        packet_size = args_map["packet-size"].as<int32_t>();
    }
    std::cout << "packet-size: " << packet_size << std::endl;
    if (packet_size <= 0)
        packet_size = DEFAULT_PACKET_SIZE;
    if (packet_size > MAX_PACKET_SIZE) {
        packet_size = MAX_PACKET_SIZE;
        std::cerr << "Warnning: packet_size = " << packet_size << " can not set to more than "
                  << MAX_PACKET_SIZE << " bytes [MAX_PACKET_SIZE]." << std::endl;
    }
    g_packet_size = packet_size;

    // thread-num
    if (args_map.count("thread-num") > 0) {
        thread_num = args_map["thread-num"].as<int32_t>();
    }
    std::cout << "thread-num: " << thread_num << std::endl;
    if (thread_num <= 0) {
        thread_num = std::thread::hardware_concurrency();
        std::cout << ">>> thread-num: std::thread::hardware_concurrency() = " << thread_num << std::endl;
    }

    // nodelay
    if (args_map.count("nodelay") > 0) {
        nodelay = args_map["nodelay"].as<std::string>();
    }
    if (nodelay == "1" || nodelay == "true") {
        g_nodelay = 1;
        g_nodelay_str = "true";
    }
    else {
        g_nodelay = 0;
        g_nodelay_str = "false";
    }
    std::cout << "TCP scoket no-delay: " << g_nodelay_str.c_str() << std::endl;

    // Run the server
    std::cout << std::endl;
    std::cout << app_name.c_str() << " begin ..." << std::endl;
    std::cout << std::endl;
    std::cout << "listen " << server_ip.c_str() << ":" << server_port.c_str() << std::endl;
    std::cout << "mode: " << mode_str.c_str() << std::endl;
    std::cout << "packet_size: " << packet_size << ", thread_num: " << thread_num << std::endl;
    std::cout << std::endl;

    if (mode == mode_http_server) {
        run_http_server(server_ip, server_port, packet_size, thread_num);
    }
    else if (mode == mode_echo_server) {
        // TODO:
        std::cout << "TODO: mode_echo_server." << std::endl;
    }
    else {
        run_http_server_ex(server_ip, server_port, packet_size, thread_num);
    }

#ifdef _WIN32
    ::system("pause");
#endif
    return 0;
}
