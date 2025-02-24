#include <conio.h>
#include <iostream>
#include "server.h"


int main()
{
    setlocale(LC_ALL, "Russian");
	try {
        boost::asio::io_context io_context;
		auto server = std::make_shared<exinity::server>(io_context, 12345);
		server->start();

        auto context_thread = std::thread(
            [&io_context]() {
				io_context.run();
			});

        // для отслеживания нажатия клавиши ESC
        while (true) {
            if (_kbhit()) {
                char ch = _getch();
                if (ch == 27) {
                    std::cout << "Exit" << std::endl;
                    server->stop();
                    break;
                }
            }
        }

        if (context_thread.joinable()) {
            context_thread.join();
        }		

	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
        return 1;
	}
    return 0;
}
