#include <SensorReads.h>

namespace SensorReads {
    //Constants
    const int NUM_SENSORS = 4; // Number of sensors
    const int DATA_SIZE = 4;   // Size of the data array for each sensor
    const int DATA_PINS[NUM_SENSORS] = {2, 4, 6, 8}; // Data pins for each sensor
    const int CLOCK_PINS[NUM_SENSORS] = {3, 5, 7, 9}; // Clock pins for each sensor
    int offsets[NUM_SENSORS] = {498503, 499203, 685284, 411793}; // Offsets for each sensor
    float scale_factors[NUM_SENSORS] = {31.852913, 31.852913, 51.050953, 50.422417}; // Scale factors for each sensor

//OBjects
    HX71708 sensors[NUM_SENSORS]; // Array of HX711 objects

    void sensor_init() {
        Serial.println("Initializing sensors...");
    // Initialize each sensor
    for (int i = 0; i < NUM_SENSORS; i++) {
        auto& sensor = sensors[i]; 
        sensor.begin(DATA_PINS[i], CLOCK_PINS[i]);
        delay(1000); // Wait for each sensor to stabilize
        sensor.set_offset(offsets[i]);
        sensor.set_scale(scale_factors[i]); // Set the scale factor for each sensor
        }
    }
}