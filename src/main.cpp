#include <Arduino.h>
#include <HX71708_ADC.h> // Die HX71708 Bibliothek für den 24-Bit A/D-Wandler
//#include "osc.h" // Die OSC Bibliothek für die Kommunikation
//#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
#include "app_config.h"
#include "sensor_runtime.h"
#include "signal_processing.h"
#include "transport_osc.h"
#include "transport_rs485.h"
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
    Serial.begin(9600);
    delay(5000);
    Serial.println("Initialisiere HX71708 ADCs...");

    initialize_sensors(sensors, AppConfig::NUM_SENSORS);
    delay(1000); // kurze Stabilisierung vor dem Tare
    warm_up_sensors(sensors, AppConfig::NUM_SENSORS, AppConfig::STARTUP_WARMUP_READS);
    calibrate_all_sensors(
        sensors,
        AppConfig::NUM_SENSORS,
        AppConfig::SCALE_FACTORS,
        AppConfig::PRE_TARE_SETTLE_MS
    );

    //calibration_values(sensors, AppConfig::NUM_SENSORS);

    // Drift-Kompensation wird automatisch bei sensor initialization gehandhabt

    Serial.println("Initialisierung abgeschlossen.");

    //WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    //WiFiAP::printPort(); // Print the UDP port number
    initialize_rs485_transport();
    delay(500);
    
    // Send initial sync frame to RS485
    send_sync_over_rs485();
}

void loop() {
    long balance = compute_balance_from_sensors(
        sensors,
        AppConfig::NUM_SENSORS,
        AppConfig::PRESENCE_THRESHOLD,
        AppConfig::BALANCE_MAX
    );
    
    //send_balance_message_osc(balance);  // OSC deaktiviert auf Arduino Nano
    send_balance_over_rs485(static_cast<int16_t>(balance));

    // Drift-Kompensation pro Sensor
    for (uint8_t i = 0; i < AppConfig::NUM_SENSORS; i++) {
        sensors[i].update_drift_compensation(balance, AppConfig::PRESENCE_THRESHOLD);
    }
}
