#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

static const uint8_t LED_BUILTIN = 8;
#define BUILTIN_LED LED_BUILTIN // backward compatibility
#define LED_BUILTIN LED_BUILTIN

static const uint8_t BOOT_BUILTIN = 9; // boot button
#define BOOT_BUILTIN BOOT_BUILTIN

#define TX1 0
#define RX1 1

static const uint8_t TX = 21;
static const uint8_t RX = 20;

static const uint8_t SDA = 8;
static const uint8_t SCL = 9;

static const uint8_t SS    = 7;
static const uint8_t MOSI  = 6;
static const uint8_t MISO  = 5;
static const uint8_t SCK   = 4;

static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A10 = 10;

#endif /* Pins_Arduino_h */