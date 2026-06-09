#include <Arduino.h>
#include <HX71708_ADC.h> // Die HX71708 Bibliothek für den 24-Bit A/D-Wandler
//#include "osc.h" // Die OSC Bibliothek für die Kommunikation
//#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
#include "app_config.h"
#include "sensor_runtime.h"
#include "signal_processing.h"
#include "bus_node.h"
// #include "drift_compensation.h"  // Drift-Kompensation ist jetzt in HX71708_ADC integriert

// Array von Sensor-Objekten
HX71708_ADC sensors[AppConfig::NUM_SENSORS] = {
    HX71708_ADC(AppConfig::SCK_PINS[0], AppConfig::DOUT_PINS[0]),
    HX71708_ADC(AppConfig::SCK_PINS[1], AppConfig::DOUT_PINS[1]),
    HX71708_ADC(AppConfig::SCK_PINS[2], AppConfig::DOUT_PINS[2]),
    HX71708_ADC(AppConfig::SCK_PINS[3], AppConfig::DOUT_PINS[3])
};

// Drift-Tracking ist jetzt in jedem HX71708_ADC-Objekt integriert

void setup() {

    Serial.begin(AppConfig::RS485_BAUD_RATE);

    SensorRuntime::initialize_sensors(sensors, AppConfig::NUM_SENSORS);
    delay(1000); // kurze Stabilisierung vor dem Tare
    SensorRuntime::warm_up_sensors(sensors, AppConfig::NUM_SENSORS, AppConfig::STARTUP_WARMUP_READS);
    SensorRuntime::calibrate_all_sensors(
        sensors,
        AppConfig::NUM_SENSORS,
        AppConfig::PRE_TARE_SETTLE_MS
    );

    BusNode::init(Serial, AppConfig::RS485_NODE_ID);
    delay(500);
}

void loop() {
    long balance = SignalProcessing::compute_balance_from_sensors(
        sensors,
        AppConfig::NUM_SENSORS,
        AppConfig::PRESENCE_THRESHOLD,
        AppConfig::BALANCE_MAX
    );

    bool system_idle = true;
    for (uint8_t i = 0; i < AppConfig::NUM_SENSORS; i++) {
        if (!sensors[i].is_idle()) {
            system_idle = false;
            break;
        }
    }

    // Node: antwortet auf Master-Anfragen mit aktuellem Balance-Wert oder leerem Status
    BusNode::poll(system_idle, balance);
}
