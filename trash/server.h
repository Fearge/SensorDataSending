#pragma once
// all Server (WebServer, WiFi) initializations and functions
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>

namespace WiFiAP {
    extern WiFiUDP Udp; // UDP instance
    extern const IPAddress remoteIP;
    extern const unsigned int remotePort; // Port, where Data is sent
    void printPort();
    void initializeAP();
}
namespace Website {
    extern WebServer server; // Web Server instance
    void initializeServer();
}
