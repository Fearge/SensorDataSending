#include "transport_rs485.h"

#include "app_config.h"

void initialize_rs485_transport() {
    // Arduino Nano: Serial nutzt automatisch Pins 0 (RX) und 1 (TX)
    // ESP32: würde Serial1 mit expliziten Pins nutzen
    Serial.begin(AppConfig::RS485_BAUD_RATE);
}
