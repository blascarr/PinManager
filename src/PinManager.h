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
	HSPI_MISO = 0x8C,
	HSPI_MOSI = 0x8D,
	HSPI_CLK = 0x8E,
	HSPI_CS = 0x8F,
	VSPI_MISO = 0x91,
	VSPI_MOSI = 0x92,
	VSPI_CLK = 0x93,
	VSPI_CS = 0x94,
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
	PinMode() : pin(0), config{.input = InputPin()}, type(PinType::None){};
	PinMode(uint8_t pin, OutputPin isOutput, PinType type,
			bool isBroken = false)
		: pin(pin), config{.output = isOutput}, mode(GPIOMode::Output),
		  isBroken(isBroken), type(type){};
	PinMode(uint8_t pin, InputPin isInput, PinType type, bool isBroken = false)
		: pin(pin), config{.input = isInput}, mode(GPIOMode::Input),
		  isBroken(isBroken), type(type){};
	void setBrokenPin(bool isBroken) { isBroken = isBroken; };
	void setGPIOMode(GPIOMode PIOMode) { mode = PIOMode; };
};

struct ESP_PinMode : public PinMode {
	bool canDeepSleep = false;
	bool canUseWithWiFi = true;
	bool isTouchGPIO = false;
	ESP_PinMode() : PinMode(){};
	ESP_PinMode(uint8_t pin, OutputPin isOutput, PinType type,
				bool canDeepSleep, bool canUseWithWiFi, bool isBroken = false)
		: PinMode(pin, isOutput, type), canDeepSleep(canDeepSleep),
		  canUseWithWiFi(canUseWithWiFi) {
		this->isBroken = isBroken;
	}
	ESP_PinMode(uint8_t pin, InputPin isInput, PinType type, bool canDeepSleep,
				bool canUseWithWiFi, bool isBroken = false)
		: PinMode(pin, isInput, type), canDeepSleep(canDeepSleep),
		  canUseWithWiFi(canUseWithWiFi) {
		this->isBroken = isBroken;
	}
};

class IPinManager {
  public:
	virtual ~IPinManager() {}
	virtual bool attach(uint8_t gpio, bool output, PinType tag) = 0;
	virtual bool attach(const PinMode *pinArray, uint8_t arrayElementCount,
						PinType tag);
	virtual bool detach(const uint8_t *pinArray, uint8_t arrayElementCount,
						PinType tag);
	virtual bool detach(uint8_t gpio);
	virtual bool detach(const PinMode *pinArray, uint8_t arrayElementCount,
						PinType tag);
	virtual bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) = 0;
	virtual bool isPinOK(uint8_t gpio) = 0;
	virtual PinType getPinType(uint8_t gpio) = 0;
};
template <typename BoardConfig>

class PinManager : public IPinManager {
  protected:
	PinMode _pins[BoardConfig::NUM_PINS];
	uint8_t pinAlloc[(BoardConfig::NUM_PINS + 7) / 8];

	struct {
		uint8_t I2CAllocCount
			: I2C_NUM_DEVICES; // allow multiple allocation of I2C bus pins
							   // but keep track of allocations
		uint8_t SPIAllocCount
			: SPI_NUM_DEVICES; // allow multiple allocation of SPI bus pins
							   // but keep track of allocations
	};

  public:
	PinManager() : I2CAllocCount(0), SPIAllocCount(0) {
		for (size_t i = 0; i < BoardConfig::NUM_PINS; ++i) {
			const auto &pinConfig = BoardConfig::PINOUT[i];
			_pins[i] = pinConfig;
		}
	}
	uint8_t getGPIOInList(uint8_t gpio) { return (gpio + 1); };
	uint8_t getPin(uint8_t gpio) { return _pins[gpio].pin; }
	bool attach(uint8_t gpio, bool output, PinType tag) {
		if (!isPinOK(gpio) || (gpio >= BoardConfig::NUM_PINS) || isI2C(tag) ||
			isSPI(tag)) {
			return false;
		}
		if (isPinAttached(gpio)) {
			return false;
		}
		uint8_t pinLocation = gpio >> 3;
		uint8_t pinIndex = gpio - 8 * pinLocation;
		bitWrite(pinAlloc[pinLocation], pinIndex, true);
		_pins[gpio] = {gpio, OutputPin(false), tag};
		return true;
	};

	bool attach(const PinMode *pinArray, uint8_t arrayElementCount,
				PinType tag) override {
		bool shouldFail = false;
		for (int i = 0; i < arrayElementCount; i++) {
			uint8_t gpio = pinArray[i].pin;
			if (gpio == 0xFF) {
				// explicit support for io -1 as a no-op (no allocation of pin),
				// as this can greatly simplify configuration arrays
				continue;
			}
			if (!isPinOK(gpio)) {
				shouldFail = true;
			}
			if (isI2C(tag) || isSPI(tag) && isPinAttached(gpio, tag)) {
				// allow multiple "allocations" of HW I2C & SPI bus pins
				continue;
			} else if (isPinAttached(gpio)) {
				shouldFail = true;
			}
		}
		if (shouldFail) {
			return false;
		}
		if (isI2C(tag))
			I2CAllocCount++;
		if (isSPI(tag))
			SPIAllocCount++;
		// all pins are available .. track each one
		for (int i = 0; i < arrayElementCount; i++) {
			uint8_t gpio = pinArray[i].pin;
			if (gpio == 0xFF) {
				// allow callers to include -1 value as non-requested pin
				// as this can greatly simplify configuration arrays
				continue;
			}
			if (gpio >= BoardConfig::NUM_PINS)
				continue; // other unexpected GPIO => avoid array bounds
						  // violation

			uint8_t by = gpio >> 3;
			uint8_t bi = gpio - 8 * by;
			bitWrite(pinAlloc[by], bi, true);
			_pins[gpio] = {gpio, OutputPin(false), tag};
		}
		return true;
	};

	bool detach(uint8_t gpio) {
		if (gpio == 0xFF)
			return true;
		if (!isPinOK(gpio))
			return false;

		if ((_pins[gpio].type != PinType::None)) {
			return false;
		}

		uint8_t pinLocation = gpio >> 3;
		uint8_t pinIndex = gpio - 8 * pinLocation;
		bitWrite(pinAlloc[pinLocation], pinIndex, false);
		_pins[gpio] = PinMode();
		return true;
	};
	bool detach(const uint8_t *pinArray, uint8_t arrayElementCount,
				PinType tag) override {
		bool shouldFail = false;
		for (int i = 0; i < arrayElementCount; i++) {
			uint8_t gpio = pinArray[i];
			if (gpio == 0xFF) {
				// explicit support for io -1 as a no-op (no allocation of pin),
				// as this can greatly simplify configuration arrays
				continue;
			}
			if (isPinAttached(gpio, tag)) {
				// if the current pin is allocated by selected owner it is
				// possible to release it
				continue;
			}
			shouldFail = true;
		}
		if (shouldFail) {
			return false; // no pins deallocated
		}
		if (isI2C(tag)) {
			if (I2CAllocCount && --I2CAllocCount > 0) {
				// no deallocation done until last owner releases pins
				return true;
			}
		}
		if (isSPI(tag)) {
			if (SPIAllocCount && --SPIAllocCount > 0) {
				// no deallocation done until last owner releases pins
				return true;
			}
		}
		for (int i = 0; i < arrayElementCount; i++) {
			detach(pinArray[i]);
		}
		return true;
	};
	bool detach(const PinMode *pinArray, uint8_t arrayElementCount,
				PinType tag) override {
		uint8_t pins[arrayElementCount];
		for (int i = 0; i < arrayElementCount; i++)
			pins[i] = pinArray[i].pin;
		return detach(pins, arrayElementCount, tag);
	};

	bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) override {
		if (!isPinOK(gpio))
			return true;
		if ((tag != PinType::None) && (_pins[gpio].type != tag))
			return false;
		if (gpio >= BoardConfig::NUM_PINS)
			return false; // catch error case, to avoid array out-of-bounds
						  // access
		uint8_t pinLocation = gpio >> 3;
		uint8_t pinIndex = gpio - (pinLocation << 3);
		return bitRead(pinAlloc[pinLocation], pinIndex);
	};
	bool isPinOK(uint8_t gpio) { return !_pins[gpio].isBroken; };
	PinType getPinType(uint8_t gpio) override {
		if (gpio >= BoardConfig::NUM_PINS)
			return PinType::None; // catch error case, to avoid array
								  // out-of-bounds
		if (!isPinOK(gpio))
			return PinType::None;
		return _pins[gpio].type;
	};
};

#endif