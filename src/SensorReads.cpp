#include <SensorReads.h>

namespace SensorReads {
//OBjects
    HX71708 sensors[NUM_SENSORS]; // Array of HX711 objects
    auto value = 0.0; // Variable to store sensor reading

    void sensor_init() {
        Serial.println("Initializing sensors...");
    // Initialize each sensor
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensors[i].begin(DATA_PINS[i], CLOCK_PINS[i], true);
        sensors[i].set_data_rate(HX71708_DATA_RATE_320); // Set data rate
        sensors[i].tare(20); // Tare the sensor
        Serial.println("Sensor " + String(i) + " initialized on Data Pin: " + String(DATA_PINS[i]) + " and Clock Pin: " + String(CLOCK_PINS[i]));

        delay(1000); // Wait for each sensor to stabilize
        }
    }
}