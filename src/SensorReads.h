#pragma once

#include <Arduino.h>
#include <HX71708.h>



//declarations
namespace SensorReads {
    //Constants
    const int NUM_SENSORS = 1; // Number of sensors
    const int DATA_SIZE =  1;   // Size of the data array for each sensor
    const int DATA_PINS[NUM_SENSORS] = {2};
    const int CLOCK_PINS[NUM_SENSORS] = {3};

    //Object Declarations
    extern HX71708 sensors[NUM_SENSORS]; // Array of HX711 objects
    
    void sensor_init();
}

