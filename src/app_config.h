#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr uint8_t NUM_SENSORS = 4;

constexpr uint8_t SCK_PINS[NUM_SENSORS] = {3, 5, 7, 9};
constexpr uint8_t DOUT_PINS[NUM_SENSORS] = {2, 4, 6, 8};

constexpr long PRESENCE_THRESHOLD = 10000;
constexpr long BALANCE_MAX = 512;

constexpr uint32_t RS485_BAUD_RATE = 115200;
constexpr uint8_t RS485_RX_PIN = 0;
constexpr uint8_t RS485_TX_PIN = 1;
constexpr uint8_t RS485_NODE_ID = 0x02; // Eindeutige ID für diesen Knoten im RS485 Netzwerk

constexpr float SCALE_FACTORS[NUM_SENSORS] = {0.0186f, 0.0186f, 0.0186f, 0.0186f};
constexpr uint8_t STARTUP_WARMUP_READS = 8;
constexpr uint16_t PRE_TARE_SETTLE_MS = 250;

// Drift-Kompensation Einstellungen
constexpr uint16_t DRIFT_IDLE_TIME_MS = 30000;      // 30s ohne Last → Drift-Korrektur
constexpr float DRIFT_WEIGHT_FACTOR = 0.2f;         // 20% neuer Offset, 80% alt (exponentiell gewichtet)
constexpr uint16_t DRIFT_COOLDOWN_MS = 5000;        // Mind. 5s zwischen Re-Tare Versuchen
constexpr uint16_t DRIFT_UPDATE_INTERVAL_MS = 1000; // Check Drift alle 1s

}