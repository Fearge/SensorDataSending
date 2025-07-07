#include <osc.h>
#include <server.h>
namespace OSC {

    //Objects 
    OSCMessage msg("/unit_"); // Create an OSC message with the address "/unit_" + unit id

    // send an OSC message over UDP
    void sendMessage(OSCMessage &msg) {
        WiFiAP::Udp.beginPacket(WiFiAP::remoteIP, WiFiAP::remotePort); // Start UDP packet
        msg.send(WiFiAP::Udp); // Send the OSC message over UDP
        WiFiAP::Udp.endPacket(); // Mark the end of the UDP packet
        msg.empty(); // Free space occupied by the message
    }
}

