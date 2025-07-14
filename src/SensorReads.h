#pragma once

#include <HX71708.h>



//declarations
namespace SensorReads {
    //Constants
    extern const int NUM_SENSORS; // Number of sensors
    extern const int DATA_SIZE1;   // Size of the data array for each sensor
    extern const int DATA_PINS[];
    extern const int CLOCK_PINS[];
    extern int offsets[]; // Offsets for each sensor
    extern float scale_factors[]; // Scale factors for each sensor

    //Object Declarations
    extern HX71708 sensors[]; // Array of HX711 objects
    
    void sensor_init();
}

