#pragma once

#include <Arduino.h>

namespace Bus {

constexpr uint8_t SOF_0 = 0xA5;
constexpr uint8_t SOF_1 = 0x5A;

enum class MessageType : uint8_t {
    SensorData = 0x01,
    Sync = 0x10,
};

struct SensorFrame {
    int16_t sensor_0;
    int16_t sensor_1;
    int16_t sensor_2;
    int16_t sensor_3;
};

struct SyncFrame {
    uint16_t slot_us;
};

struct ParsedFrame {
    MessageType type;
    uint8_t payload[8];
    uint8_t payload_size;
};

void begin(HardwareSerial &serial, uint32_t baud_rate, int8_t tx_enable_pin = -1
    , int8_t rx_pin = -1, int8_t tx_pin = -1);
void set_tx_enabled(bool enabled);

bool send_sensor_frame(const SensorFrame &frame);
bool send_sync_frame(const SyncFrame &frame);

bool read_frame(ParsedFrame &frame);
bool decode_sensor_frame(const ParsedFrame &frame, SensorFrame &out_frame);
bool decode_sync_frame(const ParsedFrame &frame, SyncFrame &out_frame);

}