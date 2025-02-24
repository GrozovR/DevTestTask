#pragma once

#include <boost/asio.hpp>
#include <random>
#include <string_view>
#include "logger.h"


namespace exinity {

using namespace boost::asio;

class client : public std::enable_shared_from_this<client> {
public:
    explicit client(io_context& io_context);
    ~client();

    void connect(const std::string& host, const std::string& port);
    void close();

private:
    io_context& context;
    ip::tcp::socket socket;
    logger logger_;
    streambuf streamBuffer;
    std::string current_message;
    // Random number generator
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

    void doConnect(const ip::tcp::resolver::results_type& endpoints);
    void doRead();
    void sendRandomNumber();
    inline short generateNextNumber()
    {
        return dist(rng);
    } 
};

} // namespace exinity
