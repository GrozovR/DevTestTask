#include "server.h"

namespace exinity {

namespace {
std::string generateUniqueFileName()
{
    using namespace std::chrono;

    std::stringstream ss;
    ss << "server_log_"
        << duration_cast<seconds>(system_clock::now().time_since_epoch())
        << ".log";
    return ss.str();
}
}


server::server(io_context& io_context, short port)
    : acceptor(io_context, ip::tcp::endpoint(ip::tcp::v4(), port)),
    timer(io_context),
    logger_(generateUniqueFileName(),"[Server]"),
    stop_flag(false)
{
    do_accept();
}

server::~server()
{
    if (dump_thread.joinable()) {
        dump_thread.join();
    }
}

void server::start()
{
    schedule_timer();
}

void server::stop()
{
    acceptor.close();
    stop_flag.store(true);
    timer.cancel();

    for (auto& session : sessions) {
        session->stop();
    }
}

int server::add_number(int number)
{
    // TODO: раздел€емый ресурс ?

    log("Received from client: " + std::to_string(number));

    if (!numbers[number]) {
        numbers[number] = true;
        sum += pow(number, 2);
    }
    return sum / numbers.size();
}

void server::log(std::string_view message)
{
    // TODO: логгер раздел€емый ресурс?
    logger_.log(message);
}

void server::do_accept()
{
    // Create a new socket for the next incoming connection
    acceptor.async_accept(
        [this](const boost::system::error_code& ec, ip::tcp::socket socket) {
            if (!ec) {
                // On successful accept, create a session for the client
                sessions.push_back(std::make_shared<session>(*this, std::move(socket)));
                sessions.back()->start();
            }
            // Accept next connection
            do_accept();
        }
    );
}

void server::schedule_timer()
{
    timer.expires_after(std::chrono::seconds(5));

    auto self = shared_from_this();
    timer.async_wait([this, self](const boost::system::error_code& ec) {
        if (!ec) {
            dump();
            if (!stop_flag) {
                schedule_timer();
            }
        } else {
            // TODO: log error ?
        }
        }
    );
}

void dump_bitset(std::bitset<1024> numbers)
{
    try {
        const size_t bytes = 128;
        static const std::string filename = "server_dump.dmp";
        std::ofstream ofs(filename, std::ios::binary | std::ios::app);
        if (!ofs) {
            // TODO: std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        // Write the bits in chunks of 8
        for (size_t byteIndex = 0; byteIndex < bytes; ++byteIndex) {
            unsigned char byteVal = 0;

            // For each bit in this byte (0..7)
            for (int bitInByte = 0; bitInByte < 8; ++bitInByte) {
                size_t bitIndex = byteIndex * 8 + bitInByte;
                if (bitIndex < numbers.size() && numbers[bitIndex]) {
                    // If that bit is set, set the corresponding bit in our byte
                    byteVal |= (1 << bitInByte);
                }
            }
            // Write this 1-byte chunk to the file
            ofs.write(reinterpret_cast<const char*>(&byteVal), 1);
        }

        ofs.close();

    } catch (std::exception& e) {

        // TODO:

    }
}

void server::dump()
{
    // ¬озможно нужно завести дампер, ибо нет возможности контролировать этот тред

    // dump_thread = std::thread(dump_bitset,numbers);
}

}