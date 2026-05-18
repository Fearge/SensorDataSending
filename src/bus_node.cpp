#include "bus_node.h"

namespace BusNode {

uint16_t crc16_ccitt(const uint8_t *data, size_t length, uint16_t init) {
    uint16_t crc = init;

    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) {
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021); //0x1021 ist das Polynom für CRC-16-CCITT
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

void build_balance_frame(uint8_t node_id, int16_t balance, uint8_t frame[FRAME_SIZE]) {
    frame[0] = FRAME_SOF;
    frame[1] = FRAME_TYPE_BALANCE;
    frame[2] = node_id;
    frame[3] = static_cast<uint8_t>(balance & 0xFF);
    frame[4] = static_cast<uint8_t>((balance >> 8) & 0xFF);

    // Escape SOF byte to prevent parser desync: if payload byte is 0xA5, replace with 0xA6
    // This sacrifices 1 unit of precision (acceptable for balance measurement)
    if (frame[3] == FRAME_SOF) {
        frame[3] = FRAME_SOF + 1;
    }
    if (frame[4] == FRAME_SOF) {
        frame[4] = FRAME_SOF + 1;
    }

    const uint16_t crc = crc16_ccitt(&frame[1], 4);
    frame[5] = static_cast<uint8_t>(crc & 0xFF);
    frame[6] = static_cast<uint8_t>((crc >> 8) & 0xFF);
}

void build_sync_frame(uint8_t node_id, uint8_t frame[FRAME_SIZE]) {
    frame[0] = FRAME_SOF;
    frame[1] = FRAME_TYPE_SYNC;
    frame[2] = node_id;
    frame[3] = 0;
    frame[4] = 0;

    const uint16_t crc = crc16_ccitt(&frame[1], 4);
    frame[5] = static_cast<uint8_t>(crc & 0xFF);
    frame[6] = static_cast<uint8_t>((crc >> 8) & 0xFF);
}

bool send_frame(HardwareSerial &port, const uint8_t frame[FRAME_SIZE]) {
    return port.write(frame, FRAME_SIZE) == FRAME_SIZE;
}

bool send_balance_frame(HardwareSerial &port, uint8_t node_id, int16_t balance) {
    uint8_t frame[FRAME_SIZE];
    build_balance_frame(node_id, balance, frame);
    return send_frame(port, frame);
}

bool send_sync_frame(HardwareSerial &port, uint8_t node_id) {
    uint8_t frame[FRAME_SIZE];
    build_sync_frame(node_id, frame);
    return send_frame(port, frame);
}

}