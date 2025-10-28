#include <Arduino.h>
#include <HX71708_ADC.h> // Die HX71708 Bibliothek für den 24-Bit A/D-Wandler
#include "osc.h" // Die OSC Bibliothek für die Kommunikation
#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
// Pin-Definitionen 
const int SCK_PINS[] = {3, 5, 7, 9};
const int DOUT_PINS[] = {2, 4, 6, 8};
const int NUM_SENSORS = 4;

const float SCALE_FACTORS[] = {0.0186, 0.0186, 0.0186, 0.0186};

// Array von Sensor-Objekten
HX71708_ADC sensors[NUM_SENSORS] = {
    HX71708_ADC(SCK_PINS[0], DOUT_PINS[0]),
    HX71708_ADC(SCK_PINS[1], DOUT_PINS[1]),
    HX71708_ADC(SCK_PINS[2], DOUT_PINS[2]),
    HX71708_ADC(SCK_PINS[3], DOUT_PINS[3])
};

void initializeSensors();
void calibrateAllSensors();
void calibrationValues();

void setup() {
    Serial.begin(115200);
    delay(5000);
    Serial.println("Initialisiere HX71708 ADCs...");

    // Initialisierung aller Sensoren mit Schleife
    initializeSensors();
    
    // Kalibrierung aller Sensoren
    calibrateAllSensors();

    //calibrationValues();
    
    Serial.println("Initialisierung abgeschlossen.");

    WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    WiFiAP::printPort(); // Print the UDP port number
}

void loop() {
    // Alle Sensoren lesen
    for (int i = 0; i < NUM_SENSORS; i++) {
        float value = sensors[i].read_corrected();
        OSC::msg.add(abs(value));
    }
    // Sende die OSC-Nachricht
    OSC::sendMessage(OSC::msg);
}

void initializeSensors() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("Initialisiere Sensor ");
        Serial.println(i + 1);
        
        sensors[i].begin();
        delay(100);
    }
}

void calibrateAllSensors() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("Kalibriere Sensor ");
        Serial.println(i + 1);
        
        sensors[i].tare();
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(" Tare abgeschlossen. Offset: ");
        Serial.println(sensors[i].get_offset());
        
        sensors[i].set_scale_factor(SCALE_FACTORS[i]);
        delay(1000);
    }
}

void calibrationValues() {
    Serial.println("Kalibrierungswerte:");
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensors[i].calibrate(1245);
        Serial.print(", Skalierungsfaktor = ");
        Serial.println(sensors[i].get_scale_factor(), 6);
    }
}

