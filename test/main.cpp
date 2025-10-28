#include <main.h>

#define RS485_RX 0  // Arduino-Empfang (an TX des RS485-Moduls)
#define RS485_TX 1 // Arduino-Senden (an RX des RS485-Moduls)

void setup(){
    Serial.begin(115200); // Initialize Serial for debugging
    //Serial1.begin(115200, SERIAL_8N1, RS485_RX, RS485_TX);
    delay(5000); // Wait for 1 second to ensure Serial is ready
    SensorReads::sensor_init(); // Initialize the sensors
    /*for (int i = 0; i < SensorReads::NUM_SENSORS; i++){
      calibrate_sensor(i, 1245); // Calibrate the first sensor with a known weight
    }*/
    WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    WiFiAP::printPort(); // Print the UDP port number
}

void loop(){
    // Read and send data for each sensor
  for (int i = 0; i < SensorReads::NUM_SENSORS; i++) {
    // Read weight from the sensor
    int gewicht = int(SensorReads::sensors[i].read()); // Average of 1 measurement
    Serial.println(gewicht);
    OSC::msg.add(gewicht); // Add the weight to the OSC message
  }
  OSC::sendMessage(OSC::msg);
}


void calibrate_sensor(int sensor_index, float known_weight) {
  auto& sensor = SensorReads::sensors[sensor_index];
  // Calibrate the sensor with a known weight
  Serial.println("Calibrating sensor ");
  Serial.println("Taring...");
  sensor.set_scale(1.0); // Set scale to 1.0 for calibration
  sensor.tare(20); // Tare the sensor
  Serial.println("offest:" + String(sensor.get_offset()));
  delay(1000); // Wait for the tare to stabilize
  Serial.print("Place a known weight on sensor ");
  Serial.println(sensor_index + 1);
  delay(5000);
  Serial.println("Done");
  sensor.calibrate_scale(known_weight, 20); // Calibrate the scale with the known weight
  Serial.println("Scale:");
  Serial.println(sensor.get_scale());
  delay(1000);
  //sensor.set_scale(44); // Set the scale factor
}