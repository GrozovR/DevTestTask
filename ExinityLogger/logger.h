#pragma once

#include <string_view>
#include <memory>
#include <mutex>
#include <queue>
#include <fstream>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace exinity {

class logger : public std::enable_shared_from_this<logger> {
public:
	logger( std::string_view fileName, std::string_view prefix);
	~logger();

	void log(std::string_view message);

private:	
	std::ofstream file_stream;
	const std::string prefix;

	std::condition_variable queue_cv;
	std::mutex queue_mutex;
	std::queue<std::string> messages;
	std::thread messages_thread;
	std::atomic<bool> stop_flag;

	void process_messages();
};

} // namespace exinity