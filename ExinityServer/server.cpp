#include "server.h"
#include <functional>

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

void dump_bitset(std::bitset<1024> numbers)
{
    try {
        static constexpr size_t bytes = 128;
        char charBitset[bytes] = { 0 };

        static const std::string filename = "server_dump.dmp";

        // Write the bits in chunks of 8
        for (size_t byteIndex = 0; byteIndex < bytes; ++byteIndex) {
            for (int bitInByte = 0; bitInByte < 8; ++bitInByte) {
                size_t bitIndex = byteIndex * 8 + bitInByte;
                if (bitIndex < numbers.size() && numbers[bitIndex]) {
                    charBitset[byteIndex] |= (1 << bitInByte);
                }
            }
        }

        std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
        if (!ofs) {
            // TODO: std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        ofs.write(charBitset, bytes);
        ofs.close();
    } catch (std::exception& e) {
        // TODO:
    }
}
}

server::server(io_context& io_context, short port)
    : acceptor(io_context, ip::tcp::endpoint(ip::tcp::v4(), port)),
    logger_(generateUniqueFileName(),"[Server]"),
    stop_flag(false)
{
}

server::~server()
{
    if (dump_thread.joinable()) {
        dump_thread.join();
    }
}

void server::start(int dump_interval)
{
    log("Started");

    do_accept();

    dump_thread = std::thread([this, dump_interval] {
        while (!stop_flag) {
            std::this_thread::sleep_for(std::chrono::seconds(dump_interval));

            std::bitset<1024> numb_copy;
            {
                std::lock_guard lock(numbers_mutex);
                numb_copy = numbers;
            }
            dump_bitset(numb_copy);

            // todo: check errors
        }
    });
}

void server::stop()
{
    log("Stopped");

    stop_flag.store(true);
    acceptor.cancel();

    for (auto& session : sessions) {
        session->stop();
    }
}

int server::add_number(int number)
{
    log("Received from client: " + std::to_string(number));

    std::lock_guard lock(numbers_mutex);

    if (!numbers[number]) {
        numbers[number] = true;
        sum += pow(number, 2);
    }
    return sum / numbers.size();
}

void server::log(std::string_view message)
{
    logger_.log(message);
}

void server::do_accept()
{
    // Create a new socket for the next incoming connection
    acceptor.async_accept(
        [this](const boost::system::error_code& ec, ip::tcp::socket socket) {
            clean_sessions();
            if (!ec) {
                log("New client connected");
                // On successful accept, create a session for the client
                sessions.push_back(std::make_shared<session>(*this, std::move(socket)));
                sessions.back()->start();
            }
            if (!stop_flag) {
                // Accept next connection
                do_accept();
            }
        }
    );
}

void server::clean_sessions()
{
    std::erase_if(sessions, [](const auto& session) {
        return session->isStoped();
        }
    );
}

}