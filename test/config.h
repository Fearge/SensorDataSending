#pragma once

#define NUM_SENSORS 2
#define DATA_SIZE 2
#define UNLOADED_THRESHOLD 200 // Threshold for unloaded weight
#define AUTO_TARE_INTERVAL 10000 // Interval for auto-taring in milliseconds

//pin definitions
extern const int DT_PINS[NUM_SENSORS]; // Data pins for each sensor
extern const int SCK_PINS[NUM_SENSORS]; // Clock pins for each sensor
