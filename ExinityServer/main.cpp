#include <conio.h>
#include <iostream>
#include "server.h"


int main()
{
    setlocale(LC_ALL, "Russian");
    std::cout << "[Server]: Started. Press ESC to exit..." << std::endl;

	try {
        boost::asio::io_context io_context;
		auto server = std::make_shared<exinity::server>(io_context, 12345);
		server->start();

        auto context_thread = std::thread(
            [&io_context]() {
				io_context.run();
			});

        // wait ESC pressing
        while (true) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 27) {
                    boost::asio::post(io_context, [&server] {
                        server->stop();
                    });
                    break;
                }
            }
        }

        if (context_thread.joinable()) {
            context_thread.join();
        }		

	} catch (std::exception& e) {
        std::cerr << "[Server] Exception: " << e.what() << std::endl;
        return 1;
	}

    std::cout << "[Server]: Stopped..." << std::endl;
    return 0;
}
