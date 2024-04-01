
#include <stddef.h>

#include "../PinManager.h"

struct Arduino_BoardConf {
	static constexpr size_t NUM_PINS = 20;
	static const PinMode PINOUT[NUM_PINS];
};

const PinMode Arduino_BoardConf::PINOUT[Arduino_BoardConf::NUM_PINS] = {
	{0, InputPin(false, false), PinType::BusDigital},
	{1, InputPin(false, false), PinType::DebugOut},
	{2, InputPin(false, true), PinType::BusDigital},
	{3, OutputPin(true), PinType::BusPWM},
	{4, OutputPin(false), PinType::BusDigital},
	{5, OutputPin(true), PinType::BusPWM},
	{6, OutputPin(false), PinType::BusDigital},
	{7, OutputPin(false), PinType::BusDigital},
	{8, OutputPin(false), PinType::BusDigital},
	{9, OutputPin(true), PinType::BusPWM},
	{10, OutputPin(true), PinType::BusPWM},
	{11, OutputPin(true), PinType::HSPI_MOSI},
	{12, OutputPin(false), PinType::HSPI_MISO},
	{13, OutputPin(false), PinType::HSPI_CLK},
	{14, InputPin(false, false), PinType::BusDigital},
	{15, InputPin(false, false), PinType::BusDigital},
	{16, InputPin(false, false), PinType::BusDigital},
	{17, InputPin(false, false), PinType::BusDigital},
	{18, InputPin(false, false), PinType::HW_I2C_SDA},
	{19, InputPin(false, false), PinType::HW_I2C_SCL}};
