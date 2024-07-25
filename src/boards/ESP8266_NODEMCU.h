#ifndef ESP8266_NODEMCU_H
#define ESP8266_NODEMCU_H

#include <stddef.h>

#include "../PinManager.h"

struct ESP8266NODEMCU_BoardConf {
	static constexpr size_t NUM_PINS = 40;
	static const ESP_PinMode PINOUT[NUM_PINS];
};

const ESP_PinMode
	ESP8266NODEMCU_BoardConf::PINOUT[ESP8266NODEMCU_BoardConf::NUM_PINS] = {
		{0, InputPin(false, false), PinType::BusDigital, false, true, false},
		{1, InputPin(false, false), PinType::DebugOut, false, true, false},
		{2, InputPin(false, true), PinType::BusDigital, false, true, false},
		{3, InputPin(false, true), PinType::BusPWM, false, true, false},
		{4, OutputPin(true), PinType::HW_I2C_SDA, false, true, false},
		{5, OutputPin(true), PinType::HW_I2C_SCL, false, true, false},
		{6, OutputPin(true), PinType::BusDigital, false, true, false},
		{7, OutputPin(true), PinType::BusDigital, false, true, false},
		{8, OutputPin(true), PinType::BusDigital, false, true, false},
		{9, OutputPin(true), PinType::BusPWM, false, true, false},
		{10, OutputPin(true), PinType::BusPWM, false, true, false},
		{11, OutputPin(true), PinType::BusDigital, false, true, false},
		{12, OutputPin(true), PinType::HSPI_MISO, false, true, false},
		{13, OutputPin(true), PinType::HSPI_MOSI, false, true, false},
		{14, InputPin(false, false), PinType::HSPI_CLK, false, true, false},
		{15, OutputPin(true), PinType::BusDigital, false, true, false},
		{16, InputPin(false, false), PinType::BusDigital, true, true, false}};
#endif