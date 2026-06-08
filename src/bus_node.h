#pragma once

#include <Arduino.h>

namespace BusNode {

constexpr uint8_t FRAME_SIZE = 7;
constexpr uint8_t FRAME_SOF = 0xA5;
constexpr uint8_t FRAME_TYPE_BALANCE = 0x01;
// Master -> Node: poll request (master asks one NodeID to reply)
constexpr uint8_t FRAME_TYPE_POLL = 0x10;

uint16_t crc16_ccitt(const uint8_t *data, size_t length, uint16_t init = 0xFFFF);
void build_balance_frame(uint8_t node_id, int16_t balance, uint8_t frame[FRAME_SIZE]);
bool send_frame(HardwareSerial &port, const uint8_t frame[FRAME_SIZE]);
bool send_balance_frame(HardwareSerial &port, uint8_t node_id, int16_t balance);

// Node runtime helpers (was rs485_node)
void init(HardwareSerial &port, uint8_t node_id);
// Call regularly from loop; pass current balance so node can reply with it
void poll(long current_balance);

}