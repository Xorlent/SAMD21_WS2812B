/**
 * SAMD21_WS2812B - A simple library for controlling WS2812B LED using the SAMD21
 * microcontroller's direct PORT register access and bit-banging using 48MHz CPU cycle 
 * counting (Arduino Zero, MKR family, Adafruit M0, etc.)
 *
 * Copyright (c) 2026 Xorlent
 * Licensed under the MIT License.
 * https://github.com/Xorlent/SAMD21_WS2812B
 */

#include "SAMD21_WS2812B.h"
#include <sam.h>     // SAMD21 peripheral register definitions (PORT, SysTick, etc.)

// WS2812B reset/cooldown: keep data line LOW (reliable at ~200us total delay w/overhead))
#define WS2812B_RESET_US  250u

// ---------------------------------------------------------------------------
// NOP-based timing - calibrated for 48 MHz
// ---------------------------------------------------------------------------
// At 48 MHz: 1 cycle ≈ 20.83 ns
//   T0H :  14 cycles  ≈  292 ns
//   T0L :  48 cycles  ≈ 1000 ns
//   T1H :  48 cycles  ≈ 1000 ns
//   T1L :  14 cycles  ≈  292 ns
// ---------------------------------------------------------------------------
// Convenience NOP macros.  asm volatile("nop") cannot be eliminated or
// reordered by the compiler and costs exactly 1 pipeline cycle on M0+.
#define NOP1   asm volatile("nop")
#define NOP2   NOP1;  NOP1
#define NOP4   NOP2;  NOP2
#define NOP8   NOP4;  NOP4
#define NOP16  NOP8;  NOP8
#define NOP32  NOP16; NOP16

// ---------------------------------------------------------------------------
// sendByte  – transmit 8 bits MSB-first with cycle-accurate PORT toggling.
//
// Marked noinline to prevent compiler optimization impacting the cycle count.
// Optimisation level O2 keeps variables in registers
// ---------------------------------------------------------------------------
__attribute__((noinline, optimize("O2")))
void WS2812B::sendByte(uint8_t data)
{
    // Local copies to avoid pointer-chasing in the hot path
    volatile uint32_t* const setReg  = _setReg;
    volatile uint32_t* const clrReg  = _clrReg;
    const uint32_t           mask    = _pinMask;

    for (uint8_t bit = 0x80u; bit != 0u; bit >>= 1u)
    {
        if (data & bit)
        {
            // ---- '1' bit ------------------------------------------------
            // T1H ≈ 1000 ns:  2 cy (OUTSET) + ~3 cy overhead + 43 NOPs = ~48 cy
            *setReg = mask;
            NOP32; NOP8; NOP2; NOP1;              // 43 NOPs

            // T1L ≈ 292 ns:  2 cy (OUTCLR) + ~3 cy overhead + 9 NOPs = ~14 cy
            *clrReg = mask;
            NOP8; NOP1;                           // 9 NOPs
        }
        else // Adds about 2 cycles, accounted for below
        {
            // ---- '0' bit ------------------------------------------------
            // T0H ≈ 292 ns:  2 cy (OUTSET) + ~5 cy overhead (branch-taken) + 7 NOPs = ~14 cy
            *setReg = mask;
            NOP4; NOP2; NOP1;                     // 7 NOPs

            // T0L ≈ 1000 ns:  2 cy (OUTCLR) + ~3 cy overhead + 43 NOPs = ~48 cy
            *clrReg = mask;
            NOP32; NOP8; NOP2; NOP1;              // 43 NOPs
        }
        // Branch back to loop top + lsrs + beq: ~5 cycles absorbed into next T_H window
    }
}

// ---------------------------------------------------------------------------
// waitAndSend – helper to enforce reset period and transmit data
// ---------------------------------------------------------------------------
inline void WS2812B::waitAndSend(uint8_t g, uint8_t r, uint8_t b)
{
    // Wait for the reset period to elapse if needed since last transmission.
    // Check here (after any color processing) so computation time counts toward reset period.
    if (_lastTransmissionMicros != 0ul) {
        unsigned long elapsed = micros() - _lastTransmissionMicros;
        if (elapsed < WS2812B_RESET_US) {
            delayMicroseconds(WS2812B_RESET_US - elapsed);
        }
    }

    // Send data - WS2812B expects GRB byte order
    // Disabling interrupts ensures that no ISR (SysTick, USB, etc.) injects
    // a mid-frame pause that the LED would interpret as a reset signal.
    noInterrupts();
    sendByte(g);
    sendByte(r);
    sendByte(b);
    interrupts();

    // Save timestamp for reset period checking on next call
    _lastTransmissionMicros = micros();
}

// ---------------------------------------------------------------------------
// Constructor / begin / destructor
// ---------------------------------------------------------------------------

WS2812B::WS2812B()
    : _setReg(nullptr), _clrReg(nullptr), _pinMask(0u), _initialized(false), _lastTransmissionMicros(0ul)
{}

bool WS2812B::begin(uint8_t pin)
{
    // Resolve the Arduino pin number to a SAMD21 PORT group + bit-mask.
    // g_APinDescription[] is provided by the SAMD21 Arduino board library
    // ulPort → 0 = PORTA, 1 = PORTB
    // ulPin  → bit position within that port group
    const uint32_t portGroup = g_APinDescription[pin].ulPort;
    const uint32_t portPin   = g_APinDescription[pin].ulPin;

    _pinMask  = (1ul << portPin);
    _setReg   = &PORT->Group[portGroup].OUTSET.reg;
    _clrReg   = &PORT->Group[portGroup].OUTCLR.reg;

    // Configure pin direction (output) and drive it low initially.
    PORT->Group[portGroup].DIRSET.reg = _pinMask;   // direction: output
    PORT->Group[portGroup].OUTCLR.reg = _pinMask;   // state: low
    // Disable the peripheral MUX for this pin so PORT controls the pad
    PORT->Group[portGroup].PINCFG[portPin].reg &=
        (uint8_t)~PORT_PINCFG_PMUXEN;

    _initialized = true;

    // Latch off cleanly before first real command
    set("black", 0);
    return true;
}

void WS2812B::set(const char* color, uint8_t brightness)
{
    if (!_initialized || !color) {
        return;
    }

    uint8_t r = 0u, g = 0u, b = 0u;

    // Use character checking for efficiency
    char first = color[0];
    
    if (first == 'r' || first == 'R') {  // "red" or "R"
        r = 255u;
    } else if (first == 'g' || first == 'G') {  // "green" or "G"
        g = 255u;
    } else if (first == 'B' || color[2] == 'u') {  // "B" or "blue"
        b = 255u;
    } else if (first == 'p') {  // "purple"
        r = 128u;
        b = 128u;
    } else if (first == 'y') {  // "yellow"
        r = 255u;
        g = 150u;
    } else if (first == 'o') {  // "orange"
        r = 255u;
        g = 75u;
    } else if (first == 'w') {  // "white"
        r = g = b = 255u;
    } else {
        // Unrecognized color or black
        waitAndSend(0u, 0u, 0u);
        return;
    }

    // Apply brightness scaling
    if (brightness == 0u) {
        waitAndSend(0u, 0u, 0u);  // Skip math entirely
        return;
    } else if (brightness < 255u) {
        r = static_cast<uint8_t>(((r * brightness) + 128) >> 8);
        g = static_cast<uint8_t>(((g * brightness) + 128) >> 8);
        b = static_cast<uint8_t>(((b * brightness) + 128) >> 8);
    }

    // Send the color data (wait + transmission + timestamp update)
    waitAndSend(g, r, b);
}