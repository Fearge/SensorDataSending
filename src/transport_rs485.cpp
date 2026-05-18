#include "transport_rs485.h"

#include "app_config.h"
#include "bus_node.h"

void initialize_rs485_transport() {
    // Arduino Nano: Serial nutzt automatisch Pins 0 (RX) und 1 (TX)
    // ESP32: würde Serial1 mit expliziten Pins nutzen
    Serial.begin(AppConfig::RS485_BAUD_RATE);
}

bool send_balance_over_rs485(int16_t balance) {
    return BusNode::send_balance_frame(Serial, AppConfig::RS485_NODE_ID, balance);
}

bool send_sync_over_rs485() {
    return BusNode::send_sync_frame(Serial, AppConfig::RS485_NODE_ID);
}