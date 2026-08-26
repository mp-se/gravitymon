#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

// WS2812 RGB LED (DIN on GPIO21)
static const uint8_t LED_BUILTIN = 21 + SOC_GPIO_PIN_COUNT;
#define BUILTIN_LED LED_BUILTIN  // backward compatibility
#define LED_BUILTIN LED_BUILTIN
#define RGB_BUILTIN LED_BUILTIN
#define RGB_BRIGHTNESS 64

static const uint8_t TX = 43;
static const uint8_t RX = 44;

static const uint8_t USB_DP = 20;
static const uint8_t USB_DM = 19;

static const uint8_t SDA = 8;
static const uint8_t SCL = 9;

static const uint8_t SS = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 13;
static const uint8_t SCK = 12;

// Left side header
static const uint8_t IO1 = 1;
static const uint8_t IO2 = 2;
static const uint8_t IO3 = 3;
static const uint8_t IO4 = 4;
static const uint8_t IO5 = 5;
static const uint8_t IO6 = 6;

// Right side header
static const uint8_t IO7 = 7;
static const uint8_t IO8 = 8;
static const uint8_t IO9 = 9;
static const uint8_t IO10 = 10;
static const uint8_t IO11 = 11;
static const uint8_t IO12 = 12;
static const uint8_t IO13 = 13;
static const uint8_t IO14 = 14;
static const uint8_t IO15 = 15;
static const uint8_t IO16 = 16;

// Bottom side pads
static const uint8_t IO17 = 17;
static const uint8_t IO18 = 18;
static const uint8_t IO38 = 38;
static const uint8_t IO39 = 39;
static const uint8_t IO40 = 40;
static const uint8_t IO41 = 41;
static const uint8_t IO42 = 42;
static const uint8_t IO45 = 45;

// ADC1 channels
static const uint8_t A0 = 1;
static const uint8_t A1 = 2;
static const uint8_t A2 = 3;
static const uint8_t A3 = 4;
static const uint8_t A4 = 5;
static const uint8_t A5 = 6;
static const uint8_t A6 = 7;
static const uint8_t A7 = 8;
static const uint8_t A8 = 9;
static const uint8_t A9 = 10;

// ADC2 channels (unavailable while WiFi is active)
static const uint8_t A10 = 17;
static const uint8_t A11 = 18;

#endif /* Pins_Arduino_h */
