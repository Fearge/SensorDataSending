#include "osc.h"
#include "config.h"

//method definitions
void sendMessage(OSCMessage &msg) {
    if (sendSerial) {
        SLIPS
        msg.send(SLIPSerial);
    } else {
        msg.send(Udp, remoteIP, remotePort);
    }
    msg.empty(); // Free space occupied by the message
}