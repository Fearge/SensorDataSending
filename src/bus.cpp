#include "bus.h"

namespace Bus {

namespace {

HardwareSerial *bus_serial = &Serial1;
int8_t tx_enable_pin = -1;

constexpr uint8_t SENSOR_PAYLOAD_SIZE = 8;
constexpr uint8_t SYNC_PAYLOAD_SIZE = 2;

struct ParserState {
	uint8_t stage = 0;
	uint8_t type = 0;
	uint8_t payload_index = 0;
	uint8_t expected_payload_size = 0;
	uint8_t payload[8] = {0};
	uint16_t computed_crc = 0;
	uint16_t expected_crc = 0;
	uint8_t crc_index = 0;
};

ParserState parser;

uint16_t crc16_ccitt_update(uint16_t crc, uint8_t data) {
	crc ^= static_cast<uint16_t>(data) << 8;
	for (uint8_t bit = 0; bit < 8; ++bit) {
		if (crc & 0x8000) {
			crc = (crc << 1) ^ 0x1021;
		} else {
			crc <<= 1;
		}
	}
	return crc;
}

uint16_t crc16_ccitt(const uint8_t *data, uint8_t size) {
	uint16_t crc = 0xFFFF;
	for (uint8_t i = 0; i < size; ++i) {
		crc = crc16_ccitt_update(crc, data[i]);
	}
	return crc;
}

bool write_u16_le(uint16_t value) {
	const uint8_t bytes[2] = {
		static_cast<uint8_t>(value & 0xFF),
		static_cast<uint8_t>((value >> 8) & 0xFF),
	};
	return bus_serial->write(bytes, sizeof(bytes)) == sizeof(bytes);
}

bool write_i16_le(int16_t value) {
	return write_u16_le(static_cast<uint16_t>(value));
}

bool send_frame(MessageType type, const uint8_t *payload, uint8_t payload_size) {
	if (bus_serial == nullptr) {
		return false;
	}

	if (tx_enable_pin >= 0) {
		digitalWrite(tx_enable_pin, HIGH);
		delayMicroseconds(5);
	}

	uint8_t crc_input[1 + 8] = {0};
	crc_input[0] = static_cast<uint8_t>(type);
	for (uint8_t i = 0; i < payload_size; ++i) {
		crc_input[1 + i] = payload[i];
	}

	const uint16_t crc = crc16_ccitt(crc_input, static_cast<uint8_t>(1 + payload_size));

	const uint8_t sof[] = {SOF_0, SOF_1, static_cast<uint8_t>(type)};
	bool ok = bus_serial->write(sof, sizeof(sof)) == sizeof(sof);
	ok = ok && (bus_serial->write(payload, payload_size) == payload_size);
	ok = ok && write_u16_le(crc);

	bus_serial->flush();

	if (tx_enable_pin >= 0) {
		delayMicroseconds(5);
		digitalWrite(tx_enable_pin, LOW);
	}

	return ok;
}

void reset_parser() {
	parser.stage = 0;
	parser.type = 0;
	parser.payload_index = 0;
	parser.expected_payload_size = 0;
	parser.computed_crc = 0;
	parser.expected_crc = 0;
	parser.crc_index = 0;
}

uint8_t expected_payload_size_for_type(uint8_t type) {
	if (type == static_cast<uint8_t>(MessageType::SensorData)) {
		return SENSOR_PAYLOAD_SIZE;
	}
	if (type == static_cast<uint8_t>(MessageType::Sync)) {
		return SYNC_PAYLOAD_SIZE;
	}
	return 0;
}

}

void begin(HardwareSerial &serial, uint32_t baud_rate, int8_t de_pin, int8_t rx_pin, int8_t tx_pin) {
	bus_serial = &serial;
	tx_enable_pin = de_pin;

	if (tx_enable_pin >= 0) {
		pinMode(tx_enable_pin, OUTPUT);
		digitalWrite(tx_enable_pin, LOW);
	}

#if defined(ARDUINO_ARCH_ESP32)
	if (rx_pin >= 0 && tx_pin >= 0) {
		bus_serial->begin(baud_rate, SERIAL_8N1, rx_pin, tx_pin);
	} else {
		bus_serial->begin(baud_rate);
	}
#else
	(void)rx_pin;
	(void)tx_pin;
	bus_serial->begin(baud_rate);
#endif

	reset_parser();
}

void set_tx_enabled(bool enabled) {
	if (tx_enable_pin < 0) {
		return;
	}
	digitalWrite(tx_enable_pin, enabled ? HIGH : LOW);
}

bool send_sensor_frame(const SensorFrame &frame) {
	uint8_t payload[8];
	const int16_t values[4] = {
		frame.sensor_0,
		frame.sensor_1,
		frame.sensor_2,
		frame.sensor_3,
	};

	for (uint8_t i = 0; i < 4; ++i) {
		payload[i * 2] = static_cast<uint8_t>(values[i] & 0xFF);
		payload[i * 2 + 1] = static_cast<uint8_t>((values[i] >> 8) & 0xFF);
	}

	return send_frame(MessageType::SensorData, payload, SENSOR_PAYLOAD_SIZE);
}

bool send_sync_frame(const SyncFrame &frame) {
	uint8_t payload[2] = {
		static_cast<uint8_t>(frame.slot_us & 0xFF),
		static_cast<uint8_t>((frame.slot_us >> 8) & 0xFF),
	};
	return send_frame(MessageType::Sync, payload, SYNC_PAYLOAD_SIZE);
}

bool read_frame(ParsedFrame &frame) {
	if (bus_serial == nullptr) {
		return false;
	}

	while (bus_serial->available() > 0) {
		const uint8_t byte = static_cast<uint8_t>(bus_serial->read());

		switch (parser.stage) {
			case 0:
				if (byte == SOF_0) {
					parser.stage = 1;
				}
				break;

			case 1:
				if (byte == SOF_1) {
					parser.stage = 2;
				} else if (byte == SOF_0) {
					parser.stage = 1;
				} else {
					reset_parser();
				}
				break;

			case 2:
				parser.type = byte;
				parser.expected_payload_size = expected_payload_size_for_type(byte);
				if (parser.expected_payload_size == 0) {
					reset_parser();
				} else {
					parser.payload_index = 0;
					parser.computed_crc = 0xFFFF;
					parser.expected_crc = 0;
					parser.computed_crc = crc16_ccitt_update(parser.computed_crc, parser.type);
					parser.stage = 3;
				}
				break;

			case 3:
				parser.payload[parser.payload_index++] = byte;
				parser.computed_crc = crc16_ccitt_update(parser.computed_crc, byte);
				if (parser.payload_index >= parser.expected_payload_size) {
					parser.crc_index = 0;
					parser.stage = 4;
				}
				break;

			case 4:
				parser.expected_crc |= static_cast<uint16_t>(byte) << (8 * parser.crc_index);
				parser.crc_index++;
				if (parser.crc_index >= 2) {
					if (parser.computed_crc == parser.expected_crc) {
						frame.type = static_cast<MessageType>(parser.type);
						frame.payload_size = parser.expected_payload_size;
						for (uint8_t i = 0; i < parser.expected_payload_size; ++i) {
							frame.payload[i] = parser.payload[i];
						}
						reset_parser();
						return true;
					}
					reset_parser();
				}
				break;

			default:
				reset_parser();
				break;
		}
	}

	return false;
}

bool decode_sensor_frame(const ParsedFrame &frame, SensorFrame &out_frame) {
	if (frame.type != MessageType::SensorData || frame.payload_size != SENSOR_PAYLOAD_SIZE) {
		return false;
	}

	out_frame.sensor_0 = static_cast<int16_t>(frame.payload[0] | (static_cast<uint16_t>(frame.payload[1]) << 8));
	out_frame.sensor_1 = static_cast<int16_t>(frame.payload[2] | (static_cast<uint16_t>(frame.payload[3]) << 8));
	out_frame.sensor_2 = static_cast<int16_t>(frame.payload[4] | (static_cast<uint16_t>(frame.payload[5]) << 8));
	out_frame.sensor_3 = static_cast<int16_t>(frame.payload[6] | (static_cast<uint16_t>(frame.payload[7]) << 8));
	return true;
}

bool decode_sync_frame(const ParsedFrame &frame, SyncFrame &out_frame) {
	if (frame.type != MessageType::Sync || frame.payload_size != SYNC_PAYLOAD_SIZE) {
		return false;
	}

	out_frame.slot_us = static_cast<uint16_t>(frame.payload[0] | (static_cast<uint16_t>(frame.payload[1]) << 8));
	return true;
}

}
