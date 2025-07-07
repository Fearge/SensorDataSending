#pragma once
#include "HX711.h"


//  adjust pins if needed.
uint8_t dataPin;
uint8_t clockPin;

namespace calibration{
    void calibrate(HX711 &myScale);
}
