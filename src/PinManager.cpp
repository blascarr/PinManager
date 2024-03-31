#include "PinManager.h"

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
bool isSPI(PinType tag) { return (is_HSPI(tag) || is_HSPI(tag)); }

template <typename BoardConfig>
bool PinManager<BoardConfig>::attach(uint8_t gpio, bool output, PinType tag) {
	if (!isPinOk(gpio, output) || (gpio >= BoardConfig::NUM_PINS) ||
		isI2C(tag) || isSPI(tag)) {
		return false;
	}
	if (isPinAttached(gpio)) {
		return false;
	}
	uint8_t pinLocation = gpio >> 3;
	uint8_t pinIndex = gpio - 8 * pinLocation;
	bitWrite(pinAlloc[pinLocation], pinIndex, true);
	_pins[gpio] = tag;

	return true;
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::detach(uint8_t gpio, PinType tag) {
	if (gpio == 0xFF)
		return true;
	if (!isPinOk(gpio, false))
		return false;

	if ((_pins[gpio] != PinType::None) && (_pins[gpio] != tag)) {
		return false;
	}

	uint8_t by = gpio >> 3;
	uint8_t bi = gpio - 8 * by;
	bitWrite(pinAlloc[by], bi, false);
	_pins[gpio] = PinType::None;
	return true;
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::attach(const PinMode *pinArray,
									 uint8_t arrayElementCount, PinType tag) {
	bool shouldFail = false;
	for (int i = 0; i < arrayElementCount; i++) {
		uint8_t gpio = pinArray[i].pin;
		if (gpio == 0xFF) {
			// explicit support for io -1 as a no-op (no allocation of pin),
			// as this can greatly simplify configuration arrays
			continue;
		}
		if (!isPinOk(gpio, pinArray[i].config.output)) {
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
	if (tisI2C(tag))
		i2cAllocCount++;
	if (isSPI(tag))
		spiAllocCount++;
	// all pins are available .. track each one
	for (int i = 0; i < arrayElementCount; i++) {
		uint8_t gpio = pinArray[i].pin;
		if (gpio == 0xFF) {
			// allow callers to include -1 value as non-requested pin
			// as this can greatly simplify configuration arrays
			continue;
		}
		if (gpio >= BoardConfig::NUM_PINS)
			continue; // other unexpected GPIO => avoid array bounds violation

		uint8_t by = gpio >> 3;
		uint8_t bi = gpio - 8 * by;
		bitWrite(pinAlloc[by], bi, true);
		_pins[gpio] = tag;
	}
	return true;
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::detach(const uint8_t *pinArray,
									 uint8_t arrayElementCount, PinType tag) {
	bool shouldFail = false;
	for (int i = 0; i < arrayElementCount; i++) {
		uint8_t gpio = pinArray[i];
		if (gpio == 0xFF) {
			// explicit support for io -1 as a no-op (no allocation of pin),
			// as this can greatly simplify configuration arrays
			continue;
		}
		if (isPinAttached(gpio, tag)) {
			// if the current pin is allocated by selected owner it is possible
			// to release it
			continue;
		}
		shouldFail = true;
	}
	if (shouldFail) {
		return false; // no pins deallocated
	}
	if (isI2C(tag)) {
		if (i2cAllocCount && --i2cAllocCount > 0) {
			// no deallocation done until last owner releases pins
			return true;
		}
	}
	if (isSPI(tag)) {
		if (spiAllocCount && --spiAllocCount > 0) {
			// no deallocation done until last owner releases pins
			return true;
		}
	}
	for (int i = 0; i < arrayElementCount; i++) {
		detach(pinArray[i], tag);
	}
	return true;
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::detach(const PinMode *pinArray,
									 uint8_t arrayElementCount, PinType tag) {
	uint8_t pins[arrayElementCount];
	for (int i = 0; i < arrayElementCount; i++)
		pins[i] = pinArray[i].pin;
	return detach(pins, arrayElementCount, tag);
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::isPinAttached(uint8_t gpio, PinType tag) {
	if (!isPinOk(gpio, false))
		return true;
	if ((tag != PinType::None) && (_pins[gpio] != tag))
		return false;
	if (gpio >= BoardConfig::NUM_PINS)
		return false; // catch error case, to avoid array out-of-bounds access
	uint8_t pinLocation = gpio >> 3;
	uint8_t pinIndex = gpio - (pinLocation << 3);
	return bitRead(pinAlloc[pinLocation], pinIndex);
}

template <typename BoardConfig>
bool PinManager<BoardConfig>::isPinOk(uint8_t gpio, bool output) {
	return true;
}

template <typename BoardConfig>
PinType PinManager<BoardConfig>::getPinType(uint8_t gpio) {
	if (gpio >= BoardConfig::NUM_PINS)
		return PinType::None; // catch error case, to avoid array out-of-bounds
							  // access
	if (!isPinOk(gpio, false))
		return PinType::None;
	return _pins[gpio];
}
