#include "sync_functions.h"
void resetAllADCsSync(HX71708_ADC* adcs[], int count) {
  // Schritt 1: Setze alle PD_SCK Pins auf OUTPUT + LOW
  for (int i = 0; i < count; ++i)
    pinMode(adcs[i]->_pdSckPin, OUTPUT);

  for (int i = 0; i < count; ++i)
    digitalWrite(adcs[i]->_pdSckPin, LOW);

  delayMicroseconds(10); // sicherstellen, dass vorher LOW war

  // Schritt 2: Pull-up (>100µs) zum Reset
  for (int i = 0; i < count; ++i)
    digitalWrite(adcs[i]->_pdSckPin, HIGH);

  delayMicroseconds(150); // laut Spezifikation: >100µs

  // Schritt 3: Zurück auf LOW → Reset abgeschlossen
  for (int i = 0; i < count; ++i)
    digitalWrite(adcs[i]->_pdSckPin, LOW);

  delayMicroseconds(10); // Kurze Pause zur Stabilisierung
}

/**
 *  @brief checkt, ob alle 4 Sensoren bereit sind
 * 
 * @return true, wenn alle Sensoren bereit sind, sonst false.
 */
bool all_ready(HX71708_ADC* adcs[], int count) {
  for (int i = 0; i < count; ++i) {
    if (digitalRead(adcs[i]->_doutPin) == HIGH)
      return false;
  }
  return true;
}

/**
 * @brief Funktion zum gleichzeitigen Auslesen aller 4 sensorwerte
 * 
 * @return die umgewandelten ADC Werte
 * 
 */
void readAllADCsSync(HX71708_ADC* adcs[], long werte[], int count) {
  // Warten, bis alle DOUT-Pins LOW sind
  unsigned long startTime = millis();
  const unsigned long timeout = 50;
  while (!all_ready(adcs, count)) {
    if (millis() - startTime > timeout) {
      Serial.println("Timeout beim Warten auf alle ADCs!");
      return;
    }
    delayMicroseconds(10);
  }

  delayMicroseconds(1); // T1: Zeit zwischen DOUT-LOW und erstem CLK

  // Lese 24 Bits synchron von allen ADCs
  for (int bit = 0; bit < 24; ++bit) {
    for (int i = 0; i < count; ++i)
      digitalWrite(adcs[i]->_pdSckPin, HIGH);
    delayMicroseconds(1);

    for (int i = 0; i < count; ++i) {
      werte[i] <<= 1;
      if (digitalRead(adcs[i]->_doutPin) == HIGH)
        werte[i]++;
    }

    for (int i = 0; i < count; ++i)
      digitalWrite(adcs[i]->_pdSckPin, LOW);
    delayMicroseconds(1);
  }

  // Zusätzliche 4 Takte für Datenratenumschaltung
  for (int j = 0; j < 4; ++j) {
    for (int i = 0; i < count; ++i)
      digitalWrite(adcs[i]->_pdSckPin, HIGH);
    delayMicroseconds(1);
    for (int i = 0; i < count; ++i)
      digitalWrite(adcs[i]->_pdSckPin, LOW);
    delayMicroseconds(1);
  }

  // Zweierkomplement-Korrektur
  for (int i = 0; i < count; ++i)
    werte[i] ^= 0x800000;
}

void readAllGramsSync(HX71708_ADC* adcs[], float grams[], int count) {
    long raw[count] = {0};

    // Rohdaten synchron lesen
    readAllADCsSync(adcs, raw, count);

    // In Gramm umrechnen und Offset anwenden
    for (int i = 0; i < count; ++i) {
      grams[i] = raw[i] - adcs[i]->get_offset(); // Offset abziehen
        grams[i] = adcs[i]->toGrams(raw[i]);
    }
}

void tareAll(HX71708_ADC* adcs[], int count) {
    long raw[count] = {0};

    readAllADCsSync(adcs, raw, count); // synchron leer auslesen

    for (int i = 0; i < count; ++i) {
        adcs[i]->set_offset(raw[i]);
    }
}
