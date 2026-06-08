#include "transport_osc.h"

#include <Arduino.h>
#include "osc.h"

void send_balance_message_osc(long balance) {
    OSC::msg.empty();
    OSC::msg.add((int32_t)balance);
    OSC::sendMessage(OSC::msg);
}