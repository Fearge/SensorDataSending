#pragma once

#include <Arduino.h>
#include <HX71708_ADC.h>

void initialize_sensors(HX71708_ADC sensors[], uint8_t num_sensors);
void warm_up_sensors(HX71708_ADC sensors[], uint8_t num_sensors, uint8_t warmup_reads);
void calibrate_all_sensors(
    HX71708_ADC sensors[],
    uint8_t num_sensors,
    const float scale_factors[],
    uint16_t pre_tare_settle_ms
);
void calibration_values(HX71708_ADC sensors[], uint8_t num_sensors);
