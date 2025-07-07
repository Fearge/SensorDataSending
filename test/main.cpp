#include "main.h"

SLIPEncodedSerial SLIPSerial(Serial); // SLIP-encoded serial communication

// Pin definitions for each sensor
const int DT_PINS[NUM_SENSORS] = {18, 23, 13, 27}; // Example pins for DOUT
const int SCK_PINS[NUM_SENSORS] = {19, 5, 14, 16}; // Example pins for SCK

HX711 sensors[NUM_SENSORS]; // Array of HX711 objects
bool shouldCalibrate = true;
int offsets[NUM_SENSORS] = {3819578, 777305, 266234, 239619}; // Offsets for each sensor
float scale_factors[NUM_SENSORS] = {40.93, 44.91,40.97, 53.28}; // Scale factors for each sensor
float data[DATA_SIZE]; // Data arrays for each sensor

bool sendSerial = false; // Flag to control sending data over serial or network
OSCMessage msg("/scale_"); // OSC address for the scale "/Adresse" "AN"

//Web Server instance
WebServer server(80); // Create a web server on port 80
char XML[2048];
bool LED0 = false; // State of the LED

// WiFi credentials
const char *ssid = "ESP32_AP"; // Access Point SSID
const char *password = "passwort"; // Access Point password

WiFiUDP Udp; // UDP instance
const IPAddress remoteIP(192, 168, 4, 2); 
const unsigned int remotePort = 8000;


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
    // Initialize the web server
  
  }
  server.on("/", sendPage); // Handle root URL
  server.on("/xml", SendXML);
  server.on("/BUTTON_0", ProcessButton_0);
  server.begin(); // Start the web server
  sensor_init(); // Initialize the sensors
  
}

void loop() {
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
    calibrate(sensors[i]); // Call the calibration function for each sensor
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
  delay(5000);
  Serial.println(sensor_index + 1);
  float reading = sensors[sensor_index].get_units(); // Get the reading
  float scale_factor = known_weight / reading; // Calculate scale factor
  sensors[sensor_index].set_scale(scale_factor); // Set the scale factor
}
void sendPage() {
  server.send(200, "text/html", PAGE_MAIN);
}
void SendXML() {

  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // show led0 status
  // show led0 status
  if (LED0) {
    strcat(XML, "<LED>1</LED>\n");
  }
  else {
    strcat(XML, "<LED>0</LED>\n");
  }


  strcat(XML, "</Data>\n");
  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above
  Serial.println(XML);

  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/xml", XML);


}
void ProcessButton_0() {

  //
  LED0 = !LED0;
  digitalWrite(LED_PIN, LED0);
  Serial.print("Button 0 "); Serial.println(LED0);
  server.send(200, "text/plain", ""); //Send web pag
  }

void calibrate(HX711 &myScale)
{
  Serial.println("\n\nCALIBRATION\n===========");
  Serial.println("remove all weight from the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("and press enter\n");
  while (Serial.available() == 0);

  Serial.println("Determine zero weight offset");
  //  average 20 measurements.
  myScale.tare(20);
  int32_t offset = myScale.get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();


  Serial.println("place a weight on the loadcell");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("enter the weight in (whole) grams and press enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n')
  {
    if (Serial.available())
    {
      char ch = Serial.read();
      if (isdigit(ch))
      {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  myScale.calibrate_scale(weight, 20);
  

  Serial.println("Scale factor: ");
  Serial.println(myScale.get_scale());
  Serial.println("\n\n");
}

