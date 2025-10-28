#pragma once

#include <OSCMessage.h>
#include <SensorReads.h>
#include <osc.h>
#include <server.h>
#include <Arduino.h>

void calibrate_sensor(int sensor_index, float known_weight);
