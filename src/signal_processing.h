#pragma once

#include <Arduino.h>
#include <HX71708_ADC.h>

long calculate_balance(long pair_0, long pair_1, long presence_threshold, long balance_max);
long compute_balance_from_sensors(
    HX71708_ADC sensors[],
    uint8_t num_sensors,
    long presence_threshold,
    long balance_max
);
