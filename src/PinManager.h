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
		isBroken = isBroken;
	}
	ESP_PinMode(uint8_t pin, InputPin isInput, PinType type, bool canDeepSleep,
				bool canUseWithWiFi, bool isBroken = false)
		: PinMode(pin, isInput, type), canDeepSleep(canDeepSleep),
		  canUseWithWiFi(canUseWithWiFi) {
		isBroken = isBroken;
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
	virtual bool isPinOk(uint8_t gpio) = 0;
	virtual PinType getPinType(uint8_t gpio) = 0;
};

template <typename BoardConfig>

class PinManager : public IPinManager {
  private:
	PinMode _pins[BoardConfig::NUM_PINS]; // Initialize to PinType:None
	uint8_t pinAlloc[(BoardConfig::NUM_PINS + 7) / 8];
	// if NUM_PINS = 7, 56bit, 1 bit per pin

	struct {
		uint8_t i2cAllocCount
			: I2C_NUM_DEVICES; // allow multiple allocation of I2C bus pins
							   // but keep track of allocations
		uint8_t spiAllocCount
			: SPI_NUM_DEVICES; // allow multiple allocation of SPI bus pins
							   // but keep track of allocations
	};

  public:
	PinManager() : i2cAllocCount(0), spiAllocCount(0) {}
	bool attach(uint8_t gpio, bool output, PinType tag) override;
	bool attach(const PinMode *pinArray, uint8_t arrayElementCount,
				PinType tag) override;
	bool detach(uint8_t gpio);
	bool detach(const uint8_t *pinArray, uint8_t arrayElementCount,
				PinType tag) override;
	bool detach(const PinMode *pinArray, uint8_t arrayElementCount,
				PinType tag) override;

	bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) override;
	virtual bool isPinOk(uint8_t gpio);
	PinType getPinType(uint8_t gpio) override;
};

#endif