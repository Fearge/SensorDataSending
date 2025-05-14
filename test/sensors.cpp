#include "sensors.h"
#include <Arduino.h>
#include "config.h"

HX711 sensors[NUM_SENSORS]; // Array of HX711 objects

//function definitions
void sensor_init() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensors[i].begin(DT_PINS[i], SCK_PINS[i]);
    calibrate_sensor(i, 1000); // Calibrate each sensor with a known weight
    Serial.print("Tariere Sensor ");
    Serial.println(i + 1);
    sensors[i].tare();
    delay(1000); // Wait 1 second for each sensor to stabilize
  }
}

void check_auto_tare() {
  static unsigned long last_tare_time = 0; // Last time the tare was performed
  unsigned long current_time = millis(); // Current time in milliseconds

  // Check if the auto-tare interval has passed
  if (current_time - last_tare_time >= AUTO_TARE_INTERVAL) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      int gewicht = int(sensors[i].get_units()); // Read weight from the sensor
      if (gewicht < UNLOADED_THRESHOLD) {
        sensors[i].tare(); // Tare the sensor if unloaded
      }
    }
    last_tare_time = current_time; // Update the last tare time
  }
}
void calibrate_sensor(int sensor_index, float known_weight) {