#include "HX711.h"

#define DT 16  // Data-Pin (DOUT) des HX711
#define SCK 17 // Clock-Pin (SCK) des HX711

HX711 scale;

void setup() {
    Serial.begin(115200);
    scale.begin(DT, SCK);
    scale.set_offset(449827); 
    scale.set_scale(-33.687500);
    
    Serial.println("Tariere...");
    scale.tare();
    delay(5000); // 5 Sekunden warten
}

void loop() {
    float gewicht = scale.get_units(10); // Mittelwert aus 10 Messungen
    
    Serial.print("Gewicht: ");
    Serial.print(gewicht, 2);
    Serial.println(" g");
    delay(100);
    
}