#pragma once
#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

SLIPEncodedSerial SLIPSerial(Serial); // SLIP-encoded serial communication
OSCMessage msg("/scale_"); // OSC address for the scale "/Adresse" "AN"

//method declarations
void sendMessage(OSCMessage &msg);