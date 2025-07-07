#pragma once

#include <OSCMessage.h>

namespace OSC {
    extern OSCMessage msg;
    void sendMessage(OSCMessage &msg);
}