#pragma once

#include "emulator/Emulator.h"

namespace common {
	class CR_Emulator : public emulator::Emulator {
	public:
		CR_Emulator(const std::string device_serial) : emulator::Emulator(device_serial) {
		}

		~CR_Emulator() {

		}

		void start_game(const std::string& app) const {
			this->start_app("com.supercell.clashroyale/com.supercell.titan.GameApp");
		}

		void stop_game(const std::string& app) const {
			this->stop_app("com.supercell.clashroyale");
		}
	};
}
