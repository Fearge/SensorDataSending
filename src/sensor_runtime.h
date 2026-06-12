#pragma once

#include <Arduino.h>
#include <HX71708_ADC.h>

namespace SensorRuntime {

void initialize_sensors(HX71708_ADC sensors[], uint8_t num_sensors);
void warm_up_sensors(HX71708_ADC sensors[], uint8_t num_sensors, uint8_t warmup_reads);
void calibrate_all_sensors(
    HX71708_ADC sensors[],
    uint8_t num_sensors,
    uint16_t pre_tare_settle_ms
);
}
