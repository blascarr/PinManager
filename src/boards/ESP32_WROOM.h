#ifndef ESP32_WROOM_H
#define ESP32_WROOM_H

#include <stddef.h>

#include "../PinManager.h"

struct ESP32WROOM_BoardConf {
	static constexpr size_t NUM_PINS = 40;
	static const ESP_PinMode PINOUT[NUM_PINS];
};

const ESP_PinMode ESP32WROOM_BoardConf::PINOUT[ESP32WROOM_BoardConf::NUM_PINS] =
	{{0, InputPin(false, false), PinType::BusDigital, true, true},
	 {1, InputPin(false, false), PinType::DebugOut, false, true},
	 {2, InputPin(false, true), PinType::BusDigital, true, true},
	 {3, InputPin(false, true), PinType::BusPWM, false, true, true},
	 {4, OutputPin(true), PinType::BusDigital, true, true},
	 {5, OutputPin(true), PinType::VSPI_CS, false, true},
	 {6, OutputPin(true), PinType::BusDigital, false, true, true},
	 {7, OutputPin(true), PinType::BusDigital, false, true, true},
	 {8, OutputPin(true), PinType::BusDigital, false, true, true},
	 {9, OutputPin(true), PinType::BusPWM, false, true, true},
	 {10, OutputPin(true), PinType::BusPWM, false, true, true},
	 {11, OutputPin(true), PinType::BusDigital, false, true, true},
	 {12, OutputPin(true), PinType::HSPI_MISO, true, true},
	 {13, OutputPin(true), PinType::HSPI_MOSI, true, true},
	 {14, InputPin(false, false), PinType::HSPI_CLK, true, true},
	 {15, OutputPin(true), PinType::BusDigital, true, true},
	 {16, InputPin(false, false), PinType::BusDigital, false, true},
	 {17, InputPin(false, false), PinType::BusDigital, false, true},
	 {18, InputPin(false, false), PinType::VSPI_CLK, false, true},
	 {19, InputPin(false, false), PinType::VSPI_MISO, false, true},
	 {20, InputPin(false, false), PinType::BusDigital, false, true},
	 {21, InputPin(false, false), PinType::HW_I2C_SDA, false, true},
	 {22, InputPin(false, true), PinType::HW_I2C_SCL, false, true},
	 {23, InputPin(false, true), PinType::VSPI_MOSI, false, true},
	 {24, OutputPin(true), PinType::BusDigital, false, true},
	 {25, OutputPin(true), PinType::BusPWM, true, true},
	 {26, OutputPin(true), PinType::BusDigital, true, true},
	 {27, OutputPin(true), PinType::BusDigital, true, true},
	 {28, OutputPin(true), PinType::BusDigital, false, true},
	 {29, OutputPin(true), PinType::BusPWM, false, true},
	 {30, OutputPin(true), PinType::BusPWM, false, true},
	 {31, OutputPin(true), PinType::BusDigital, false, true},
	 {32, OutputPin(true), PinType::BusDigital, true, true},
	 {33, OutputPin(true), PinType::BusDigital, true, true},
	 {34, InputPin(false, true), PinType::BusDigital, true, true},
	 {35, InputPin(false, true), PinType::BusDigital, true, true},
	 {36, InputPin(false, true), PinType::BusDigital, true, true},
	 {37, InputPin(false, false), PinType::BusDigital, false, true},
	 {38, InputPin(false, false), PinType::BusDigital, false, true},
	 {39, InputPin(false, true), PinType::BusDigital, true, true}};
#endif