#include <Arduino.h>
#include <HX71708_ADC.h> // Die HX71708 Bibliothek für den 24-Bit A/D-Wandler
#include "osc.h" // Die OSC Bibliothek für die Kommunikation
#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
#include "app_config.h"
#include "sensor_runtime.h"
#include "signal_processing.h"
#include "transport_osc.h"

// Array von Sensor-Objekten
HX71708_ADC sensors[AppConfig::NUM_SENSORS] = {
    HX71708_ADC(AppConfig::SCK_PINS[0], AppConfig::DOUT_PINS[0]),
    HX71708_ADC(AppConfig::SCK_PINS[1], AppConfig::DOUT_PINS[1]),
    HX71708_ADC(AppConfig::SCK_PINS[2], AppConfig::DOUT_PINS[2]),
    HX71708_ADC(AppConfig::SCK_PINS[3], AppConfig::DOUT_PINS[3])
};

void setup() {
    Serial.begin(115200);
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

    Serial.println("Initialisierung abgeschlossen.");

    WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    WiFiAP::printPort(); // Print the UDP port number
}

void loop() {
    long balance = compute_balance_from_sensors(
        sensors,
        AppConfig::NUM_SENSORS,
        AppConfig::PRESENCE_THRESHOLD,
        AppConfig::BALANCE_MAX
    );
    send_balance_message_osc(balance);
}
