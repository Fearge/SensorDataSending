#pragma once
#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

SLIPEncodedSerial SLIPSerial(Serial); // SLIP-encoded serial communication

//method declarations
void sendMessage(OSCMessage &msg);