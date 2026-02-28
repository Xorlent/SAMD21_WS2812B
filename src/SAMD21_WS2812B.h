/**
 * SAMD21_WS2812B - A simple library for controlling WS2812B LED using the SAMD21
 * microcontroller's direct PORT register access and bit-banging using 48MHz CPU cycle 
 * counting (Arduino Zero, MKR family, Adafruit M0, etc.)
 *
 * Copyright (c) 2026 Xorlent
 * Licensed under the MIT License.
 * https://github.com/Xorlent/SAMD21_WS2812B
 */

#ifndef SAMD21_WS2812B_H
#define SAMD21_WS2812B_H

#include <Arduino.h>
#include <stdint.h>

class WS2812B {
public:
    WS2812B();

    // Initialize with an Arduino pin number.
    // Configures the pin as output and caches the PORT register pointers.
    // Returns true on success.
    bool begin(uint8_t pin);

    // Set color with optional brightness (1–255, 255 = full(default)).
    // Colors: "black", "white", "red", "green", "blue", "purple", "yellow", "orange"
    void set(const char* color, uint8_t brightness = 255);

private:
    // Cached pointers to the SAMD21 PORT OUTSET / OUTCLR registers and the
    // pin's bitmask within that port group.  Computed in begin() so the
    // hot path in sendByte() is register-only.
    volatile uint32_t *_setReg;
    volatile uint32_t *_clrReg;
    uint32_t           _pinMask;
    bool               _initialized;
    unsigned long      _lastTransmissionMicros;

    // Transmit one byte MSB-first using cycle-accurate PORT toggling.
    // Must be called with interrupts disabled for reliability.
    void sendByte(uint8_t data);
};

#endif // SAMD21_WS2812B_H
