#include "pch.h"
#include "logger.h"
#include <iostream>
#include <chrono>

namespace exinity {

logger::logger(std::string_view filename, std::string_view _prefix)
	: file_stream(filename.data()/*todo: open mode */, std::ios::app),
	prefix(_prefix), stop_flag(false)
{
	if (!file_stream.is_open()) {
		throw std::runtime_error("Can't open log file.");
	}

	messages_thread = std::thread([this](){ process_messages(); });
}

logger::~logger()
{
	stop_flag.store(true);
	queue_cv.notify_one();

	if (messages_thread.joinable()) {
		messages_thread.join();
	}
}

void logger::log(std::string_view message)
{
	logMessage(prefix, message);
}

void logger::logError(std::string_view error)
{
	logMessage("[Error]", error);
}

void logger::logMessage(std::string_view prefix, std::string_view message)
{
	std::stringstream messageStr;
	messageStr << prefix.data()
		<< std::chrono::system_clock::now()
		<< " " << message << std::endl;

	std::lock_guard lock(queue_mutex);
	messages.push(messageStr.str());

	queue_cv.notify_one();
}

void logger::process_messages()
{
	while (true) {
		std::unique_lock lock(queue_mutex);
		queue_cv.wait_for(lock, std::chrono::seconds(5),
			[this] { return !messages.empty() || stop_flag; });

		while (!messages.empty()) {
			file_stream << messages.front() << std::endl;
			std::cout << messages.front() << std::endl;
			messages.pop();
		}
		file_stream.flush();

		if (stop_flag) {
			break;
		}
	}
}
} // namespace exinity
