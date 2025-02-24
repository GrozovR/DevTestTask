#include "client.h"

// TODO:!
#include <iostream>

namespace exinity {

using namespace boost::asio;

namespace {
std::string generateUniqueFileName()
{
    using namespace std::chrono;

    std::stringstream ss;
    ss << "client_log_"
        << duration_cast<seconds>(system_clock::now().time_since_epoch())
        << ".log";
    return ss.str();
}
}

client::client(io_context& io_context)
    : context(io_context),
    socket(io_context),
    logger_(generateUniqueFileName(), "[Client]"),
    rng(std::random_device{}()),
    dist(0, 1023)
{
}

client::~client()
{
}

void client::connect(const std::string& host, const std::string& port)
{
    ip::tcp::resolver resolver(context);
    auto endpoints = resolver.resolve(host, port);

    // Start an async connect
    doConnect(endpoints);
}

void client::close()
{
    // TODO:

    //logFile.close();
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

void client::doConnect(const ip::tcp::resolver::results_type& endpoints)
{
    auto self(shared_from_this());
    async_connect(
        socket,
        endpoints,
        [this, self](const boost::system::error_code& ec, const ip::tcp::endpoint& /*endpoint*/) {
            if (!ec) {
                log("Connected to server.");
                sendRandomNumber();
            } else {
                log( "Connect failed: " + ec.message() );
                // TODO: можно сделать несколько попыток
            }
        }
    );
}

void client::doRead()
{
    if (!socket.is_open()) {
        std::cerr << "Socket is not open!" << std::endl;
        return;
    }

    auto self(shared_from_this());
    async_read_until(
        socket,
        streamBuffer,
        '\n',
        [this, self](const boost::system::error_code& ec, std::size_t /*bytes_transferred*/) {
            if (!ec) {
                std::istream is(&streamBuffer);
                std::string line;
                std::getline(is, line);
                log("Received from server: " + line);

                sendRandomNumber();

            } else {
                std::cerr << "[Client] Read error: " << ec.message() << std::endl;
                // TODO: может быть переотрыть сокет ?
                socket.close();
            }
        }
    );
}


void client::sendRandomNumber()
{
    if (!socket.is_open()) {
        std::cerr << "Socket is not open!" << std::endl;
        return;
    }

    // Generate a random integer
    short number = generateNextNumber();
    current_message = std::to_string(number) + "\n";

    log("Sending: " + current_message);

    auto self(shared_from_this());
    async_write(socket, buffer(current_message),
        [this, self](const boost::system::error_code& ec, std::size_t /*bytes_transferred*/) {
            if (!ec) {
                doRead();
            } else {
                std::cerr << "Send error: " << ec.message() << std::endl;
                // TODO: может быть переотрыть сокет ?
                socket.close();
            }
        }
    );
}

void client::log(std::string_view message)
{
    std::stringstream messageStr;
    messageStr << std::chrono::system_clock::now() << " " << message;
    logger_.log(messageStr.str());
}

} // namespace exinity