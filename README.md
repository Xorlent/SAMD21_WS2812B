# Minimal RGB LED Library for WS2812B on SAMD21

The simplest, most lightweight WS2812B RGB LED library for SAMD21-based Arduino boards (Arduino Zero, MKR family, Adafruit M0, etc.). For single RGB control, this library **saves program, memory space, and CPU cycles** compared to popular RGB LED control libraries.

## Features

- **Ultra-minimal** - Uses direct PORT register access and cycle-accurate bit-banging
- **Simple API** - Just two functions: `begin()` and `set()`
- **8 predefined colors** - Common colors built-in

## Hardware Connection

Connect your WS2812B LED to your SAMD21 board:

```
WS2812B Pin  →  SAMD21
-----------     ------
VDD (5V)     →  5V or VIN
VSS (GND)    →  GND
DIN          →  Any GPIO pin (e.g., pin 6)
```

## Installation

1. Download or clone this repository
2. Move the `SAMD21_WS2812B` folder to your Arduino libraries folder:
   - **macOS**: `~/Documents/Arduino/libraries/`
   - **Windows**: `Documents\Arduino\libraries\`
   - **Linux**: `~/Arduino/libraries/`
3. Restart Arduino IDE

## Usage

```cpp
#include <SAMD21_WS2812B.h>

#define LED_PIN 6  // GPIO pin for WS2812B data (DIN)

WS2812B led;

void setup() {
  // Initialize with GPIO pin number
  led.begin(LED_PIN);
}

void loop() {
  // Flash red with a 1 second interval
  led.set("red", 255); // Brightness value optional, defaults to full brightness
  delay(1000);
  led.set("black");
  delay(1000);
}
```

## API Reference

### `bool begin(uint8_t pin)`

Initialize the WS2812B controller on the specified GPIO pin.

- **Parameters**: `pin` - GPIO pin number (e.g., 5)
- **Returns**: `true` on success, `false` on failure

### `void set(const char* color, uint8_t brightness = 255)`

Set the LED color and brightness.

- **Parameters**:
  - `color` - Color name (see below)
  - `brightness` - Brightness level from 1-255 (255 = full brightness)
    - Use "black" to turn the LED off
- **Returns**: None

### Supported Colors

| Color String | RGB Values | Alternative |
|-------------|------------|-------------|
| `"black"`   | Off        | -           |
| `"white"`   | 255,255,255| -           |
| `"red"`     | 255,0,0    | `"R"`       |
| `"green"`   | 0,255,0    | `"G"`       |
| `"blue"`    | 0,0,255    | `"B"`       |
| `"purple"`  | 128,0,128  | -           |
| `"yellow"`  | 255,255,0  | -           |
| `"orange"`  | 255,165,0  | -           |

## Timing

The library uses CPU cycle-accurate bit-banging calibrated for **48 MHz SAMD21** boards. Interrupts are disabled during transmission to prevent timing jitter.

## Requirements

- **SAMD21-based Arduino board** (Arduino Zero, MKR Zero, MKR1000, Adafruit Feather M0, Adafruit M0 Express, etc.)
- **Single WS2812B LED**

## Troubleshooting

**LED doesn't light up:**
- Check power connections (WS2812B needs both 5V and GND)
- Verify data pin connection
- Ensure your board runs at 48 MHz (standard for SAMD21)
- Try a different GPIO pin

**LED shows wrong colors:**
- Some WS2812B variants use RGB instead of GRB - check your LED's datasheet
- If needed, change the byte order in SAMD21_WS2812B.cpp:
```cpp
    // Standard WS2812B expects GRB order
    sendByte(g);
    sendByte(r);
    sendByte(b);
```

**Timing issues or flickering:**
- This library is calibrated for 48 MHz SAMD21. Other clock speeds would require adjusting the NOP counts
- Ensure no high-priority interrupts are running frequently (USB, timers, etc.)

**Compilation errors:**
- Ensure your board is properly configured in Arduino IDE (Tools > Board > Arduino SAMD Boards)