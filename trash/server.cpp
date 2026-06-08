#include "server.h"


//Objects for WiFi and WebServer

namespace WiFiAP {
    WiFiUDP Udp; // UDP instance
    const char *ssid = "ESP32_AP"; // Access Point SSID
    const char *password = "passwort"; // Access Point password
    const IPAddress remoteIP(192, 168, 4, 2);
    const unsigned int remotePort = 8000; //Port, where Data is sent
    void initializeAP() {
    // Initialize WiFi in AP mode
        Serial.println("Starting Access Point...");
        WiFi.softAP(ssid, password);
        Serial.println("Access Point started");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    }
    void printPort() {
        // Get the port number for UDP communication
        Serial.print("UDP Port: ");
        Serial.println(remotePort);
    }
}

namespace Website {
    WebServer server(80); // Create a web server on port 80

    void initializeServer() {
        // TODO: Website initialization
    }
}