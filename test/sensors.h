#pragma once
#include <HX711.h>

extern HX711 sensors[];
void sensor_init();
void check_auto_tare();
void calibrate_sensor(int sensor_index, float known_weight);