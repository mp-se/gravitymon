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

static const uint8_t IO0 = 0;
static const uint8_t IO1 = 1;
static const uint8_t IO2 = 2;
static const uint8_t IO3 = 3;
static const uint8_t IO4 = 4;
static const uint8_t IO5 = 5;
static const uint8_t IO6 = 6;

static const uint8_t IO7 = 7;
static const uint8_t IO8 = 8;
static const uint8_t IO9 = 9;
static const uint8_t IO10 = 10;
static const uint8_t IO20 = 20;
static const uint8_t IO21 = 21;

#endif /* Pins_Arduino_h */
