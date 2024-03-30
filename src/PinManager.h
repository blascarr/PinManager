#ifndef _PIN_MANAGER_H
#define _PIN_MANAGER_H

#include <Arduino.h>
#include <stdint.h>
#define I2C_NUM_DEVICES 4 // 2⁴ = 16 I2C Devices
#define SPI_NUM_DEVICES 4 // 2⁴ = 16 SPI Devices

enum struct PinType : uint8_t {
	None = 0, // default == legacy == unspecified owner
	Ethernet = 0x81,
	BusDigital = 0x82,
	BusOnOff = 0x83,
	BusPwm = 0x84,	 // 'BusP' == PWM output using BusPwm
	Button = 0x85,	 // 'Butn' == button from configuration
	IR = 0x86,		 // 'IR'   == IR receiver pin from configuration
	Relay = 0x87,	 // 'Rly'  == Relay pin from configuration
	SPI_RAM = 0x88,	 // 'SpiR' == SPI RAM
	DebugOut = 0x89, // 'Dbg'  == debug output always IO1
	DMX = 0x8A,		 // 'DMX'  == hard-coded to IO2
	HW_I2C = 0x8B,
	// 'I2C'  == hardware I2C pins (4&5 on ESP8266, 21&22 on ESP32)
	HW_SPI = 0x8C, // 'SPI'  == hardware (V)SPI pins (13,14&15 on ESP8266,
				   // 5,18&23 on ESP32)
};
static_assert(0u == static_cast<uint8_t>(PinType::None),
			  "PinType::None must be zero, so default array initialization "
			  "works as expected");

typedef struct PinMode {
	int8_t pin;
	bool isOutput;
	bool isPullup;
	PinType type;
};

class IPinManager {
  public:
	virtual ~IPinManager() {}
	virtual bool attach(uint8_t gpio, bool output, PinType tag) = 0;
	virtual bool attach(const PinMode *pinArray, uint8_t arrayElementCount,
						PinType tag);
	virtual bool detach(uint8_t gpio, PinType tag);
	virtual bool detach(const uint8_t *pinArray, uint8_t arrayElementCount,
						PinType tag);
	virtual bool detach(const PinMode *pinArray, uint8_t arrayElementCount,
						PinType tag);
	inline void detach(uint8_t gpio) { detach(gpio, PinType::None); }

	virtual bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) = 0;
	virtual bool isPinOk(uint8_t gpio, bool output = true) = 0;
	virtual PinType getPinType(uint8_t gpio) = 0;
};

template <typename BoardConfig>

class PinManager : public IPinManager {
  private:
	PinType _pins[BoardConfig::NUM_PINS] = {
		PinType::None}; // Initialize to PinType:None
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
	bool detach(uint8_t gpio, PinType tag) override;
	bool detach(const uint8_t *pinArray, uint8_t arrayElementCount,
				PinType tag) override;
	bool detach(const PinMode *pinArray, uint8_t arrayElementCount,
				PinType tag) override;

	bool isPinAttached(uint8_t gpio, PinType tag = PinType::None) override;
	virtual bool isPinOk(uint8_t gpio, bool output = true);
	PinType getPinType(uint8_t gpio) override;
};

#endif