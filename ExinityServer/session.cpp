#include "session.h"
#include "server.h"

namespace exinity{

session::session(server & _server, ip::tcp::socket _socket)
    : socket(std::move(_socket)), server_ref(_server)
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
        // Not all sockets support shutdown (e.g., already closed or not connected).
        // // You can log or handle the error, but it's often fine to ignore.
    }
    socket.close(ec);
    if (ec) {
        // Handle or log if the close failed (rare).
    }
}

void session::doRead()
{
    auto self(shared_from_this());
    // Read data asynchronously into buffer_
    async_read_until(
        socket, buffer, '\n',
        [this, self](const boost::system::error_code& ec, std::size_t /*bytes_transferred*/) {
            if (!ec) {
                // Extract message from the buffer
                std::istream is(&buffer);

                std::string num;
                std::getline(is, num);

                answer = std::to_string(server_ref.add_number(std::stoi(num))) + '\n';

                //server.log( answer);
                //std::cout << "Received from client: " << num << std::endl;

                // Echo it back or process further
                doWrite();

            } else {
                server_ref.log( "Read from client error: " + ec.message() );
                // Handle error (e.g., connection closed)
                // TODO: ?
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
                // Handle error
                // TODO: ?
            }
        }
    );
}

}