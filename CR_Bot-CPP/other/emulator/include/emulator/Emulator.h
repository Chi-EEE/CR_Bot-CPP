#pragma once

#include "av/CodecContext.hpp"

namespace emulator {
	class Emulator {
	private:
		const std::string device_serial;
		mutable cv::Mat last_frame;

	public:
		Emulator(const std::string device_serial) : device_serial(device_serial) {}

		~Emulator() {}

		void click(int x, int y) const;

		void start_app(const std::string& app) const;

		void stop_app(const std::string& app) const;

		[[nodiscard]]
		std::pair<int, int> get_size(const std::string device_serial) const;

		[[nodiscard]]
		cv::Mat screenshot() const;

		void start_record() const;

		void run_adb_command(const std::vector<std::string> commands) const;

		[[nodiscard]]
		std::string run_adb_command_with_output(const std::vector<std::string> commands) const;
	};
}
