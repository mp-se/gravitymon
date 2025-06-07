#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "soc/soc_caps.h"

// BUILTIN_LED can be used in new Arduino API digitalWrite() like in Blink.ino
static const uint8_t LED_BUILTIN = 8;
#define BUILTIN_LED LED_BUILTIN // backward compatibility
#define LED_BUILTIN LED_BUILTIN // allow testing #ifdef LED_BUILTIN

static const uint8_t TX = 21;
static const uint8_t RX = 20;

// Fallback I2C pins
static const uint8_t SCL = 0;
static const uint8_t SDA = 1;

static const uint8_t GPIO0 = 0;
static const uint8_t GPIO1 = 1;
static const uint8_t GPIO2 = 2;
static const uint8_t GPIO3 = 3;
static const uint8_t GPIO4 = 4;
static const uint8_t GPIO5 = 5;
static const uint8_t GPIO6 = 6;

static const uint8_t GPIO7 = 7;
static const uint8_t GPIO8 = 8;
static const uint8_t GPIO9 = 9;
static const uint8_t GPIO10 = 10;
static const uint8_t GPIO20 = 20;
static const uint8_t GPIO21 = 21;

#endif /* Pins_Arduino_h */
