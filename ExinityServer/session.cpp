#include "session.h"
#include "server.h"

namespace exinity{

session::session(server & _server, ip::tcp::socket _socket)
    : socket(std::move(_socket)),
    server_ref(_server), is_stoped( false )
{
}

session::~session()
{
    server_ref.log("Session closed");
}

void session::start()
{
    doRead();
}

void session::stop()
{
    boost::system::error_code ec;
    socket.shutdown(ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        server_ref.log("Read from client error: " + ec.message());
    }
    socket.close(ec);
    if (ec) {
        server_ref.log("Read from client error: " + ec.message());
    }
    is_stoped = true;
}

void session::doRead()
{
    auto self(shared_from_this());
    async_read_until(
        socket, buffer, '\n',
        [this, self](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                // Extract message from the buffer
                std::istream is(&buffer);
                std::string num;
                std::getline(is, num);

                int avg = server_ref.add_number(std::stoi(num));
                answer = std::to_string(avg) + '\n';
                doWrite();
            } else {
                server_ref.log("Read from client error: " + ec.message());
                stop();
            }
        }
    );
}

void session::doWrite()
{
    auto self(shared_from_this());
    boost::asio::async_write(
        socket, boost::asio::buffer(answer),
        [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (!ec) {
                // After writing, we can read again
                server_ref.log("Write to client: " + answer);
                doRead();
            } else {
                server_ref.log("Write to client error: " + ec.message());
                stop();
            }
        }
    );
}

}