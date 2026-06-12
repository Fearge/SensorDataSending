#pragma once

#include <Arduino.h>

namespace BusNode {

constexpr uint8_t FRAME_SIZE = 7;
constexpr uint8_t FRAME_SOF = 0xA5;
constexpr uint8_t FRAME_TYPE_BALANCE = 0x01;
constexpr uint8_t FRAME_TYPE_EMPTY = 0x02;
// Master -> Node: poll request (master asks one NodeID to reply)
constexpr uint8_t FRAME_TYPE_POLL = 0x10;

uint16_t _crc16_ccitt(const uint8_t *data, size_t length, uint16_t init = 0xFFFF);
void _build_balance_frame(uint8_t node_id, int16_t balance, uint8_t frame[FRAME_SIZE]);
bool _send_frame(HardwareSerial &port, const uint8_t frame[FRAME_SIZE]);
bool _send_balance_frame(HardwareSerial &port, uint8_t node_id, int16_t balance);
bool send_empty_frame(HardwareSerial &port, uint8_t node_id);
void init(HardwareSerial &port, uint8_t node_id);
// Call regularly from loop; pass unloaded state and balance value so node can reply accordingly
void poll(bool system_idle, long balance);

}