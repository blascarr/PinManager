#ifndef _PIN_MANAGER_H
#define _PIN_MANAGER_H

#include <Arduino.h>
#include <stdint.h>
#define I2C_NUM_DEVICES 4 // 2⁴ = 16 I2C Devices
#define SPI_NUM_DEVICES 4 // 2⁴ = 16 SPI Devices

enum class GPIOMode { Input, Output, Undefined };

enum struct PinType : uint8_t {
	None = 0, // default == legacy == unspecified owner
	Ethernet = 0x81,
	BusDigital = 0x82,
	BusOnOff = 0x83,
	BusPWM = 0x84,	   // 'BusP' == PWM output using BusPwm
	Button = 0x85,	   // 'Butn' == button from configuration
	IR = 0x86,		   // 'IR'   == IR receiver pin from configuration
	Relay = 0x87,	   // 'Rly'  == Relay pin from configuration
	SPI_RAM = 0x88,	   // 'SpiR' == SPI RAM
	DebugOut = 0x89,   // 'Dbg'  == debug output always IO1
	DMX = 0x8A,		   // 'DMX'  == hard-coded to IO2
	HW_I2C_SDA = 0x8B, // 'I2C'  == hardware I2C pins
	HW_I2C_SCL = 0x8C, // 'SPI' hardware(H) Virtual (V)SPI pins
	HSPI_MISO = 0x8D,
	HSPI_MOSI = 0x8E,
	HSPI_CLK = 0x8F,
	HSPI_CS = 0x91,
	VSPI_MISO = 0x92,
	VSPI_MOSI = 0x93,
	VSPI_CLK = 0x94,
	VSPI_CS = 0x95,
};

static_assert(0u == static_cast<uint8_t>(PinType::None),
			  "PinType::None must be zero, so default array initialization "
			  "works as expected");

bool isI2C(PinType tag) {
	return (tag == PinType::HW_I2C_SDA || tag == PinType::HW_I2C_SCL);
};

bool is_HSPI(PinType tag) {
	return (tag == PinType::HSPI_MISO || tag == PinType::HSPI_MOSI ||
			tag == PinType::HSPI_CLK || tag == PinType::HSPI_CS);
}
bool is_VSPI(PinType tag) {
	return (tag == PinType::VSPI_MISO || tag == PinType::VSPI_MOSI ||
			tag == PinType::VSPI_CLK || tag == PinType::VSPI_CS);
}
bool isSPI(PinType tag) { return (is_HSPI(tag) || is_VSPI(tag)); }

struct OutputPin {
	bool isPWM = false;
	OutputPin(bool pwm = false) : isPWM(pwm) {}
};

struct InputPin {
	bool isPullup = false;
	bool isInputOnly = false;
	bool isInterrupt = false;
	InputPin(bool pullup = false, bool isInputOnly = false,
			 bool interrupt = false)
		: isPullup(pullup), isInputOnly(isInputOnly), isInterrupt(interrupt) {}
};

struct PinMode {
	uint8_t pin;
	GPIOMode mode = GPIOMode::Undefined;
	union {
		InputPin input;
		OutputPin output;
	} config;
	bool isBroken = false;
	bool isADC = false;
	bool isDAC = false;
	PinType type;
	PinMode() : pin(0), config{.input = InputPin()}, type(PinType::None) {};
	PinMode(uint8_t pin, OutputPin isOutput, PinType type,
			bool isBroken = false)
		: pin(pin), config{.output = isOutput}, mode(GPIOMode::Output),
		  isBroken(isBroken), type(type) {};
	PinMode(uint8_t pin, InputPin isInput, PinType type, bool isBroken = false)
		: pin(pin), config{.input = isInput}, mode(GPIOMode::Input),
		  isBroken(isBroken), type(type) {};
	void setBrokenPin(bool isBroken) { isBroken = isBroken; };
	void setGPIOMode(GPIOMode PIOMode) { mode = PIOMode; };
};

struct ESP_PinMode : public PinMode {
	bool canDeepSleep = false;
	bool canUseWithWiFi = true;
	bool isTouchGPIO = false;
	ESP_PinMode() : PinMode() {};
	ESP_PinMode(uint8_t pin, OutputPin isOutput, PinType type,
				bool canDeepSleep = false, bool canUseWithWiFi = true,
				bool isBroken = false)
		: PinMode(pin, isOutput, type), canDeepSleep(canDeepSleep),
		  canUseWithWiFi(canUseWithWiFi) {
		this->isBroken = isBroken;
	}
	ESP_PinMode(uint8_t pin, InputPin isInput, PinType type,
				bool canDeepSleep = false, bool canUseWithWiFi = true,
				bool isBroken = false)
		: PinMode(pin, isInput, type), canDeepSleep(canDeepSleep),
		  canUseWithWiFi(canUseWithWiFi) {
		this->isBroken = isBroken;
	}
};

struct DefaultBoardConfig {
	static constexpr uint8_t NUM_PINS = 10;
	static ESP_PinMode PINOUT[NUM_PINS];
	static constexpr uint8_t INITIAL_PINS = 0;
};

class IPinManager {
  public:
	virtual ~IPinManager() {}
	virtual bool detach(const uint8_t *pinArray, uint8_t arrayElementCount);
	virtual bool detach(uint8_t gpio);
	virtual bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) = 0;
	virtual bool isPinOK(uint8_t gpio) = 0;
	virtual PinType getPinType(uint8_t gpio) = 0;
};
template <typename BoardConfig = DefaultBoardConfig,
		  typename PinModeConf = PinMode>

class PinManager : public IPinManager {
  protected:
	PinModeConf _pins[BoardConfig::NUM_PINS];
	uint8_t pinAlloc[(BoardConfig::NUM_PINS + 7) / 8];

	struct {
		uint8_t I2CAllocCount
			: I2C_NUM_DEVICES; // allow multiple allocation of I2C bus pins
							   // but keep track of allocations
		uint8_t SPIAllocCount
			: SPI_NUM_DEVICES; // allow multiple allocation of SPI bus pins
							   // but keep track of allocations
	};

	void gpioBitWrite(uint8_t gpio, bool status = false) {
		uint8_t pinLocation = gpio >> 3;
		uint8_t pinIndex = gpio - 8 * pinLocation;
		bitWrite(pinAlloc[pinLocation], pinIndex, status);
	}
	bool pinNotSupported(uint8_t gpio) {
		if (gpio >= BoardConfig::NUM_PINS)
			return true;
		return false;
	}

  public:
	PinManager() : I2CAllocCount(0), SPIAllocCount(0) {
		for (size_t i = 0; i < BoardConfig::INITIAL_PINS; ++i) {
			const auto &pinConfig = BoardConfig::PINOUT[i];
			if (!pinConfig.isBroken) {
				attach(pinConfig);
			}
		}
		memset(pinAlloc, 0, sizeof(pinAlloc));
	}

	uint8_t getPin(uint8_t gpio) { return _pins[gpio].pin; }
	PinModeConf getGPIO(uint8_t gpio) { return _pins[gpio]; }
	void setGPIOMode(uint8_t pin, GPIOMode PIOMode) {
		if (PIOMode == GPIOMode::Input) {
			pinMode(pin, INPUT);
		}
		if (PIOMode == GPIOMode::Output) {
			pinMode(pin, OUTPUT);
		}
	}

	bool attach(uint8_t gpio) {
		if (isPinAttached(gpio) || pinNotSupported(gpio)) {
			return false;
		}
		gpioBitWrite(gpio, true);
		PinModeConf pinConfig = PinModeConf();
		pinConfig.pin = gpio;
		_pins[gpio] = pinConfig;
		// By default is OUTPUT
		setGPIOMode(pinConfig.pin, GPIOMode::Output);
		return true;
	}

	bool attach(PinModeConf pinConfig) {
		if (isPinAttached(pinConfig.pin)) {
			return false;
		}
		uint8_t gpio = pinConfig.pin;
		gpioBitWrite(gpio, true);
		_pins[gpio] = pinConfig;
		setGPIOMode(pinConfig.pin, pinConfig.mode);
		return true;
	}

	bool detach(uint8_t gpio) {
		if (pinNotSupported(gpio))
			return false;
		gpioBitWrite(gpio, false);
		_pins[gpio] = PinModeConf();
		return true;
	};

	bool attach(const PinModeConf *pinArray, uint8_t arrayElementCount) {
		for (int i = 0; i < arrayElementCount; i++) {
			PinModeConf pinConfig = pinArray[i];
			attach(pinConfig);
		}
		return true;
	};

	bool detach(const PinModeConf *pinArray, uint8_t arrayElementCount) {
		for (int i = 0; i < arrayElementCount; i++) {
			detach(pinArray[i].pin);
		}
		return true;
	};

	bool detach(const uint8_t *pinArray, uint8_t arrayElementCount) override {
		for (int i = 0; i < arrayElementCount; i++) {
			detach(pinArray[i]);
		}
		return true;
	};

	bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) override {
		if (pinNotSupported(gpio) ||
			((tag != PinType::None) && (_pins[gpio].type != tag)))
			return false;
		if (!isPinOK(gpio))
			return true;
		uint8_t pinLocation = gpio >> 3;
		uint8_t pinIndex = gpio - (pinLocation << 3);
		return bitRead(pinAlloc[pinLocation], pinIndex);
	};
	bool isPinOK(uint8_t gpio) { return !_pins[gpio].isBroken; };
	PinType getPinType(uint8_t gpio) override {
		if (pinNotSupported(gpio) || !isPinOK(gpio))
			return PinType::None;
		return _pins[gpio].type;
	};
};

#endif