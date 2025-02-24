#pragma once

#include <boost/asio.hpp>
#include "session.h"
#include <bitset>
#include <string_view>
#include <memory>
#include <fstream>
#include "logger.h"
#include <thread>
#include <atomic>

namespace exinity {

using namespace boost::asio;

class server : public std::enable_shared_from_this<server>
{
public:
    server(io_context& io_context, short port);
    ~server();

    void start();
    void stop();

    int add_number(int number);
    
    void log(std::string_view message);

private:
    
    ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<session>> sessions;

    std::thread dump_thread;
    steady_timer timer;

    std::bitset<1024> numbers;
    int sum = 0;

    logger logger_;

    std::atomic<bool> stop_flag;

    void do_accept();
    void schedule_timer();
    void dump();
};

}
 