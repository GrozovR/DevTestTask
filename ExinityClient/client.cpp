#include "client.h"

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
    boost::system::error_code ec;
    socket.shutdown(ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        logger_.logError(ec.message());
    }
    socket.close(ec);
    if (ec) {
        logger_.logError(ec.message());
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
                logger_.log("Connected to server.");
                sendRandomNumber();
            } else {
                logger_.logError( "Connect failed: " + ec.message() );
                // TODO: it's posible to try reconnect
            }
        }
    );
}

void client::doRead()
{
    if (!socket.is_open()) {
        logger_.logError("Socket is not open!");
        return;
    }

    auto self(shared_from_this());
    async_read_until(
        socket,
        streamBuffer,
        '\n',
        [this, self](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                std::istream is(&streamBuffer);
                std::string line;
                std::getline(is, line);
                logger_.log("Received from server: " + line);

                sendRandomNumber();

            } else {
                logger_.logError("Reading error: " + ec.message());
                close();
            }
        }
    );
}

void client::sendRandomNumber()
{
    if (!socket.is_open()) {
        logger_.logError("Socket is not open!");
        return;
    }

    // Generate a random integer
    short number = generateNextNumber();
    current_message = std::to_string(number) + "\n";

    logger_.log("Sending: " + current_message);

    auto self(shared_from_this());
    async_write(socket, buffer(current_message),
        [this, self](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                doRead();
            } else {
                logger_.logError( "Sending error: " + ec.message());
                close();
            }
        }
    );
}

} // namespace exinity