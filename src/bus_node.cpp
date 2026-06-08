#include "bus_node.h"

namespace BusNode {

static HardwareSerial *serialPort = nullptr;
static uint8_t this_node_id = 0;

static void build_frame(uint8_t msg_type, uint8_t node_id, uint8_t payload_lsb, uint8_t payload_msb, uint8_t frame[FRAME_SIZE]) {
    frame[0] = FRAME_SOF;
    frame[1] = msg_type;
    frame[2] = node_id;
    frame[3] = payload_lsb;
    frame[4] = payload_msb;

    const uint16_t crc = crc16_ccitt(&frame[1], 4);
    frame[5] = static_cast<uint8_t>(crc & 0xFF);
    frame[6] = static_cast<uint8_t>((crc >> 8) & 0xFF);
}

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
    const uint8_t payload_lsb = static_cast<uint8_t>(balance & 0xFF);
    const uint8_t payload_msb = static_cast<uint8_t>((balance >> 8) & 0xFF);

    build_frame(FRAME_TYPE_BALANCE, node_id, payload_lsb == FRAME_SOF ? FRAME_SOF + 1 : payload_lsb,
                payload_msb == FRAME_SOF ? FRAME_SOF + 1 : payload_msb, frame);
}

bool send_frame(HardwareSerial &port, const uint8_t frame[FRAME_SIZE]) {
    return port.write(frame, FRAME_SIZE) == FRAME_SIZE;
}

bool send_balance_frame(HardwareSerial &port, uint8_t node_id, int16_t balance) {
    uint8_t frame[FRAME_SIZE];
    build_balance_frame(node_id, balance, frame);
    return send_frame(port, frame);
}

void init(HardwareSerial &port, uint8_t node_id) {
    serialPort = &port;
    this_node_id = node_id;
}

// Simple read helper that blocks briefly to collect bytes
static bool read_exact(uint8_t *buf, size_t len, unsigned long timeout_ms = 10) {
    unsigned long start = millis();
    size_t idx = 0;
    while (idx < len && (millis() - start) < timeout_ms) {
        if (serialPort->available()) {
            int b = serialPort->read();
            if (b >= 0) buf[idx++] = (uint8_t)b;
        }
    }
    return idx == len;
}

void poll(long current_balance) {
    if (!serialPort) return;

    while (serialPort->available()) {
        int b = serialPort->read();
        if (b < 0) break;
        if ((uint8_t)b != FRAME_SOF) {
            continue; // wait for SOF
        }

        uint8_t rest[6];
        if (!read_exact(rest, 6)) {
            break; // incomplete
        }

        uint8_t msg_type = rest[0];
        uint8_t node_id = rest[1];
        uint8_t payload_lsb = rest[2];
        uint8_t payload_msb = rest[3];
        uint16_t frame_crc = (uint16_t)rest[4] | ((uint16_t)rest[5] << 8);

        uint8_t crc_in[4] = { msg_type, node_id, payload_lsb, payload_msb };
        uint16_t computed = crc16_ccitt(crc_in, 4);
        if (computed != frame_crc) {
            // bad crc, ignore
            continue;
        }

        if (msg_type == FRAME_TYPE_POLL) {
            // Master asking to poll; respond only if addressed to this node
            if (node_id == this_node_id) {
                // send current balance as int16 immediately (no backoff)
                int16_t bal16 = (int16_t)current_balance;
                send_balance_frame(*serialPort, this_node_id, bal16);
            }
        }
        // ignore other frame types for node
    }
}

}