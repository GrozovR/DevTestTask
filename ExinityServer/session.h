#pragma once

#include <boost/asio.hpp>


namespace exinity {

using namespace boost::asio;

class server;

class session : public std::enable_shared_from_this<session>
{
public:
    session(server& _server, ip::tcp::socket _socket);
    ~session();

    void start();
    void stop();

    bool isStoped() { return is_stoped; }

private:
    ip::tcp::socket socket;
    std::string answer;
    streambuf buffer;
    server& server_ref;
    bool is_stoped;

    void doRead();
    void doWrite();
};
}