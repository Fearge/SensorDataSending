#include <main.h>

void setup(){
    Serial.begin(115200); // Initialize Serial for debugging
    delay(5000); // Wait for 1 second to ensure Serial is ready
    SensorReads::sensor_init(); // Initialize the sensors
    calibrate_sensor(0, 1245); // Calibrate the first sensor with a known weight of 100 grams
    WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    WiFiAP::printPort(); // Print the UDP port number
}

void loop(){
    unsigned long startTime = millis(); // Start time
    // Read and send data for each sensor
  for (int i = 0; i < SensorReads::NUM_SENSORS; i++) {
    // Read weight from the sensor
    float gewicht = SensorReads::sensors[i].read(); // Average of 1 measurement
    OSC::msg.add(gewicht); // Add the weight to the OSC message
  }
  OSC::sendMessage(OSC::msg);
  unsigned long endTime = millis(); // End time
    Serial.print("Loop runtime (ms): ");
    Serial.println(endTime - startTime);

}


void calibrate_sensor(int sensor_index, float known_weight) {
  // Calibrate the sensor with a known weight
  SensorReads::sensors[sensor_index].set_scale(1.0); // Set scale to 1.0 for calibration
  SensorReads::sensors[sensor_index].tare(20); // Tare the sensor
  delay(1000); // Wait for the tare to stabilize
  Serial.print("Place a known weight on sensor ");
  delay(5000);
  Serial.println(sensor_index + 1);
  float reading = SensorReads::sensors[sensor_index].get_units(); // Get the reading
  float scale_factor = known_weight / reading; // Calculate scale factor
  SensorReads::sensors[sensor_index].set_scale(44.91); // Set the scale factor
}