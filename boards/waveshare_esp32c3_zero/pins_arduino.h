#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

static const uint8_t LED_BUILTIN = 10+SOC_GPIO_PIN_COUNT;;
#define BUILTIN_LED LED_BUILTIN // backward compatibility
#define LED_BUILTIN LED_BUILTIN
#define RGB_BUILTIN LED_BUILTIN
#define RGB_BRIGHTNESS 64

static const uint8_t TX = 21;
static const uint8_t RX = 20;

static const uint8_t USB_DP = 19;
static const uint8_t USB_DM = 18;

static const uint8_t SDA = 1;
static const uint8_t SCL = 0;

static const uint8_t S10 = 10; // Also used for RGB LED
static const uint8_t S9 = 9;
static const uint8_t S8 = 8;
static const uint8_t S7 = 7;
static const uint8_t S6 = 6;
static const uint8_t S5 = 5;

static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;

#endif /* Pins_Arduino_h */