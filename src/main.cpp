#include <Arduino.h>
#include <OSCMessage.h>
#include <OSCBoards.h>
#include <WiFi.h>	
#include <WiFiUdp.h>
#include <HX711.h>
#include <SLIPEncodedSerial.h>
#include <WebServer.h>

#define NUM_SENSORS 2 // Number of sensors
#define DATA_SIZE 2   // Size of the data array for each sensor
#define UNLOADED_THRESHOLD 200 // Threshold for unloaded weight
#define AUTO_TARE_INTERVAL 10000 // Interval for auto-taring in milliseconds

//method declarations
void sendMessage(OSCMessage &msg);
void sensor_init();
void check_auto_tare();
void calibrate_sensor(int sensor_index, float known_weight);
void handleRoot();

SLIPEncodedSerial SLIPSerial(Serial); // SLIP-encoded serial communication

// Pin definitions for each sensor
const int DT_PINS[NUM_SENSORS] = {16, 25}; // Example pins for DOUT
const int SCK_PINS[NUM_SENSORS] = {17, 26}; // Example pins for SCK

HX711 sensors[NUM_SENSORS]; // Array of HX711 objects
int offsets[NUM_SENSORS] = {449827, 777348}; // Offsets for each sensor
float scale_factors[NUM_SENSORS] = {-33.687500, -47.082603}; // Scale factors for each sensor
float data[DATA_SIZE]; // Data arrays for each sensor

bool sendSerial = false; // Flag to control sending data over serial or network
OSCMessage msg("/scale_"); // OSC address for the scale

//Web Server instance
WebServer server(80); // Create a web server on port 80

// WiFi credentials
const char *ssid = "ESP32_AP"; // Access Point SSID
const char *password = "12345678"; // Access Point password

WiFiUDP Udp; // UDP instance
const IPAddress remoteIP(192, 168, 4, 2); // Replace with the IP of the receiving device
const unsigned int remotePort = 8000; // Replace with the port of the receiving device


void setup() {
  Serial.begin(115200);     // Initialize Serial for debugging
  if(sendSerial){
    SLIPSerial.begin(115200); // Initialize SLIPSerial
  }else{
    // Initialize WiFi in AP mode
    WiFi.softAP(ssid, password);
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
  // Initialize the web server
  server.on("/", handleRoot); // Handle root URL
  server.begin(); // Start the web server

  sensor_init(); // Initialize the sensors
}

void loop() {
  check_auto_tare(); // Check if auto-tare is needed
  // Read and send data for each sensor
  for (int i = 0; i < NUM_SENSORS; i++) {
    // Read weight from the sensor
    int gewicht = int(sensors[i].get_units()); // Average of 1 measurement
    msg.add(gewicht); // Add the weight to the OSC message
  }
  sendMessage(msg);  
  server.handleClient(); // Handle incoming web requests
}

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
void sendMessage(OSCMessage &msg) {
  if (sendSerial) {
    SLIPSerial.beginPacket();
    msg.send(SLIPSerial); // Send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // Mark the end of the OSC Packet
    msg.empty(); // Free space occupied by the message
  }else{
    Udp.beginPacket(remoteIP, remotePort); // Start UDP packet
    msg.send(Udp); // Send the OSC message over UDP
    Udp.endPacket(); // Mark the end of the UDP packet
    msg.empty(); // Free space occupied by the message
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
  // Calibrate the sensor with a known weight
  sensors[sensor_index].set_scale(1.0); // Set scale to 1.0 for calibration
  sensors[sensor_index].tare(); // Tare the sensor
  delay(1000); // Wait for the tare to stabilize
  Serial.print("Place a known weight on sensor ");
  Serial.println(sensor_index + 1);
  float reading = sensors[sensor_index].get_units(); // Get the reading
  float scale_factor = known_weight / reading; // Calculate scale factor
  sensors[sensor_index].set_scale(scale_factor); // Set the scale factor
}
void handleRoot() {
  server.send(200, "text/html", "<h1<Hello from ESP32!</h1>");
}