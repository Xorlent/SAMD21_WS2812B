/*
 * SAMD21_WS2812B Simple Example
 *
 * Copyright (c) 2026 Xorlent
 * Licensed under the MIT License.
 * https://github.com/Xorlent/SAMD21_WS2812B
 *
 * Demonstrates the SAMD21_WS2812B library on any SAMD21 board @ 48MHz
 * (Arduino Zero, MKR family, Adafruit Feather M0, Adafruit M0 Express, etc.).
 *
 * Connect your WS2812B LED data pin to the Arduino pin defined by LED_PIN.
 * Change LED_PIN to whichever GPIO is wired to the LED Data In (DIN) on your board.
 *
 * The LED will cycle through all available colors at different brightness levels.
 */

#include <SAMD21_WS2812B.h>

#define LED_PIN 11   // Change to the GPIO pin connected to WS2812B DIN

WS2812B led;

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait for Serial to be ready
  Serial.println("Starting WS2812B test...");

  if (led.begin(LED_PIN)) {
    Serial.println("WS2812B initialized");
  } else {
    Serial.println("Failed to initialize WS2812B RGB LED");
    while (1);  // Halt
  }
}

void loop() {
  Serial.println("Red");
  led.set("red");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("red", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("red", i);
    delay(20);
  }

  Serial.println("Green");
  led.set("green");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("green", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("green", i);
    delay(20);
  }

  Serial.println("Blue");
  led.set("blue");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("blue", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("blue", i);
    delay(20);
  }

  Serial.println("Orange");
  led.set("orange");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("orange", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("orange", i);
    delay(20);
  }

  Serial.println("Purple");
  led.set("purple");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("purple", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("purple", i);
    delay(20);
  }

  Serial.println("Yellow");
  led.set("yellow");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("yellow", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("yellow", i);
    delay(20);
  }

  Serial.println("White");
  led.set("white");
  delay(1000);

  for(int i=1;i<180;i++){
    led.set("white", i);
    delay(20);
  }

  for(int i=180;i>0;i--){
    led.set("white", i);
    delay(20);
  }

  Serial.println("Off (black)");
  led.set("black");
  delay(2000);
}