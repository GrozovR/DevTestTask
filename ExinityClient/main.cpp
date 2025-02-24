#include <conio.h>
#include <iostream>
#include "client.h"

int main()
{
    setlocale(LC_ALL, "Russian");
    std::cout << "[Client]: Started. Press ESC to exit..." << std::endl;

    try {
        boost::asio::io_context context;
        auto client = std::make_shared<exinity::client>(context);
		client->connect("localhost","12345");

        auto context_thread = std::thread(
            [&context]() {
                context.run();
            });

        while (true) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 27) {
                    // NO !
                    std::cout << "Exit" << std::endl;
                    client->close();
                    break;
                }
            }
        }
        if (context_thread.joinable()) {
            context_thread.join();
        }

    } catch (std::exception& e) {
        std::cerr << "Client exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[Client]: Stopped..." << std::endl;
    return 0;
}
