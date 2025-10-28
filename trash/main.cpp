#include <Arduino.h>
#include "HX71708_ADC.h" // Bindet die Klassendeklaration ein
#include "osc.h" // Die OSC Bibliothek für die Kommunikation
#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
#include "sync_functions.h"
HX71708_ADC adc1(2, 3);
HX71708_ADC adc2(4, 5);
HX71708_ADC adc3(6, 7);
HX71708_ADC adc4(8, 9);

HX71708_ADC* adcs[4] = { &adc1, &adc2, &adc3, &adc4 };

void setup() {
    // Synchroner Reset
    resetAllADCsSync(adcs, 4);
    delay(15); // 4 Datenzyklen warten

    // Taren (Offset setzen)
    tareAll(adcs, 4);

    // Kalibrieren
    adc1.set_scale_factor(0.0186); // Beispielwerte
    adc2.set_scale_factor(0.0186);
    adc3.set_scale_factor(0.0186);
    adc4.set_scale_factor(0.0186);
}

void loop() {
    float werteGramm[4] = {0};
    readAllGramsSync(adcs, werteGramm, 4);

    for (int i = 0; i < 4; ++i) {
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(werteGramm[i], 2);
        Serial.println(" g");
    }

    delay(100);
}
