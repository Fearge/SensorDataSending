#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr uint8_t NUM_SENSORS = 4;

constexpr uint8_t SCK_PINS[NUM_SENSORS] = {3, 5, 7, 9};
constexpr uint8_t DOUT_PINS[NUM_SENSORS] = {2, 4, 6, 8};

constexpr long PRESENCE_THRESHOLD = 10000;
constexpr long BALANCE_MAX = 512;

constexpr float SCALE_FACTORS[NUM_SENSORS] = {0.0186f, 0.0186f, 0.0186f, 0.0186f};
constexpr uint8_t STARTUP_WARMUP_READS = 8;
constexpr uint16_t PRE_TARE_SETTLE_MS = 250;

}