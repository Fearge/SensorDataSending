#pragma once

#include <OSCMessage.h>

namespace OSC {
    extern OSCMessage msg;
    void sendMessage(OSCMessage &msg);
    void sendFloatArray(float* data, int arraySize);
}