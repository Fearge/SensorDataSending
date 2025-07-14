#pragma once

#include <Arduino.h>
#include <OSCMessage.h>
#include <OSCBoards.h>
#include <WiFi.h>	
#include <WiFiUdp.h>
#include <HX711.h>
#include <SLIPEncodedSerial.h>
#include <WebServer.h>
#include "SuperMon.h"

#define NUM_SENSORS 4 // Number of sensors
#define DATA_SIZE 4   // Size of the data array for each sensor
#define UNLOADED_THRESHOLD 200 // Threshold for unloaded weight
#define AUTO_TARE_INTERVAL 10000 // Interval for auto-taring in milliseconds

#define LED_PIN 2


//method declarations
void sendMessage(OSCMessage &msg);
void sensor_init();
void check_auto_tare();
void calibrate_sensor(int sensor_index, float known_weight);
void sendPage();
void ProcessButton_0();
void SendXML();
void calibrate(HX711 &myScale);