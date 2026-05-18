#pragma once

#include <Arduino.h>

void initialize_rs485_transport();
bool send_balance_over_rs485(int16_t balance);
bool send_sync_over_rs485();