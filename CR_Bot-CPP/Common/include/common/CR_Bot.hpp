#pragma once

#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <tl/expected.hpp>

#include "CR_Emulator.hpp"

#include "state/GameState.hpp"
#include "state/ActionCard.hpp"

namespace common {
	class CR_Bot {
	public:
		const CR_Emulator emulator;
		std::chrono::steady_clock::time_point last_run_time;

	public:
		CR_Bot(std::string device_serial) : emulator(CR_Emulator(device_serial)) {
		}

		void step(std::function<std::optional<state::ActionCard>(const state::GameState&)> callback) {
			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_run_time);
			if (elapsed.count() < 10) {
				return;
			}
			this->last_run_time = now;
			std::optional<state::ActionCard> maybe_action = callback(state::GameState{});
			if (maybe_action.has_value()) {
				//this->emulator.play_card(maybe_action.value());
			}
		}

		void start_record() {
			std::thread record_thread(&CR_Emulator::start_record, &this->emulator);
		}
	};
}
