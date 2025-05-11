#include <Arduino.h>
#include <OSCMessage.h>
#include <OSCBoards.h>
#include <HX711.h>
#include <SLIPEncodedSerial.h>

#define NUM_SENSORS 4 // Number of sensors
#define DATA_SIZE 4   // Size of the data array for each sensor

// Pin definitions for each sensor
const int DT_PINS[NUM_SENSORS] = {16, 18, 20, 22}; // Example pins for DOUT
const int SCK_PINS[NUM_SENSORS] = {17, 19, 21, 23}; // Example pins for SCK

HX711 sensors[NUM_SENSORS]; // Array of HX711 objects
int offsets[NUM_SENSORS] = {449827, 449827, 449827, 449827}; // Offsets for each sensor
float scale_factors[NUM_SENSORS] = {-33.687500, -33.687500, -33.687500, -33.687500}; // Scale factors for each sensor

int data[DATA_SIZE]; // Data arrays for each sensor

SLIPEncodedSerial SLIPSerial(Serial); // SLIP-encoded serial communication

void setup() {
  SLIPSerial.begin(115200); // Initialize SLIPSerial
  Serial.begin(115200);     // Initialize Serial for debugging

  // Initialize each sensor
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensors[i].begin(DT_PINS[i], SCK_PINS[i]);
    sensors[i].set_offset(offsets[i]);
    sensors[i].set_scale(scale_factors[i]);
    Serial.print("Tariere Sensor ");
    Serial.println(i + 1);
    sensors[i].tare();
    delay(1000); // Wait 1 second for each sensor to stabilize
  }
}

void loop() {
  // Read and send data for each sensor
  for (int i = 0; i < NUM_SENSORS; i++) {
    // Read weight from the sensor
    float gewicht = sensors[i].get_units(10); // Average of 1 measurement
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" Gewicht: ");
    Serial.println(gewicht);

    /* Populate the data array with dummy values (replace with actual logic if needed)
    for (int j = 0; j < DATA_SIZE; j++) {
      data[i][j] = gewicht + j; // Example: Add incremental values
    }*/
    data[i] = gewicht; // Store the weight in the data array
  }
    // Create and send OSC message
  OSCMessage msg("/scale_"); // OSC address for the scale
  for (int j = 0; j < DATA_SIZE; j++) {
    msg.add(data[j]); // Add data to the OSC message
  
  }
  sendMessage(msg);

  delay(100); // Delay between loops
}

void sendMessage(OSCMessage &msg) {
  SLIPSerial.beginPacket();
  msg.send(SLIPSerial); // Send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // Mark the end of the OSC Packet
  msg.empty(); // Free space occupied by the message
}