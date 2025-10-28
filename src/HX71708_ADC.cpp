#include "HX71708_ADC.h" // Bindet die Klassendeklaration ein

/**
 * @brief Konstruktor für die HX71708_ADC-Klasse.
 * @param pdSckPin Der GPIO-Pin, der mit dem PD_SCK-Pin des HX71708 verbunden ist.
 * @param doutPin Der GPIO-Pin, der mit dem DOUT-Pin des HX71708 verbunden ist.
 */
HX71708_ADC::HX71708_ADC(int pdSckPin, int doutPin) {
    _pdSckPin = pdSckPin;
    _doutPin = doutPin;
    _offset = 0; // Initialisiere den Offset auf 0
    _scale_factor = 1.0; // Initialisiere den Skalierungsfaktor auf 1.0
}

long HX71708_ADC::get_offset(void) {
    return _offset;
}
void HX71708_ADC::set_offset(long offset) {
        _offset = offset;
    }

void HX71708_ADC::set_scale_factor(float scale_factor) {
    _scale_factor = scale_factor;
}


float HX71708_ADC::get_scale_factor(void) {
    return _scale_factor;
}

float HX71708_ADC::toGrams(long raw) {
        return (raw - _offset) * _scale_factor;
}

/**
 * @brief Initialisiert den HX71708 ADC nach dem Einschalten.
 *        Konfiguriert die GPIO-Pins und setzt den ADC in einen bekannten Zustand.
 *        Diese Methode sollte einmalig für jede Instanz beim Start des Systems aufgerufen werden.
 */
void HX71708_ADC::begin(void) {
    // Konfiguriere die Pins als OUTPUT bzw. INPUT [1, 2]
    pinMode(_pdSckPin, OUTPUT);
    pinMode(_doutPin, INPUT);

    // Zu Beginn PD_SCK 100 Mikrosekunden auf HIGH setzen, um den ADC zurückzusetzen
    digitalWrite(_pdSckPin, HIGH);
    delayMicroseconds(150); // Eine Verzögerung von 150us ist größer als die geforderten 100us [3].
    digitalWrite(_pdSckPin, LOW);
    delay(100); // mindestens 4 Datenzyklen warten
}

/**
 * @brief Liest einen 24-Bit Sensorwert vom HX71708 ADC und setzt die nächste Datenrate auf 320Hz.
 *        Die serielle Kommunikation erfolgt über die Pins PD_SCK und DOUT.
 *        Die 24-Bit-Ausgangsdaten sind im binären Zweierkomplement-Code formatiert,
 *        wobei das MSB das Vorzeichenbit ist.
 *        Die Umwandlung in einen vorzeichenlosen Wert erfolgt durch XOR mit 0x800000.
 * @return Der umgewandelte 24-Bit Wert. Gibt 0 zurück, falls ein Timeout auftritt
 *         und der ADC zurückgesetzt werden muss.
 *         Nach einem Reset oder einer Änderung der Datenrate benötigt der ADC
 *         vier Datenzyklen, um stabile Ausgangsdaten zu liefern.
 */
long HX71708_ADC::read320Hz(void) {
    unsigned char i;
    unsigned long bcd = 0; // Speichert den 24-Bit internen Code

    // Stelle sicher, dass PD_SCK zunächst LOW ist, bevor auf DOUT gewartet wird
    digitalWrite(_pdSckPin, LOW);

    // Warten, bis DOUT auf Low geht. Dies signalisiert, dass der A/D-Wandler bereit ist, Daten auszugeben
    // Wenn DOUT nicht Low geht, kann dies auf ein Problem hinweisen, und der ADC muss zurückgesetzt werden
    unsigned long startTime = millis();
    // Ein Timeout von 50ms ist angemessen, da bei 320Hz ein Datenzyklus ca. 3.125ms dauert.
    // Dies gibt dem ADC ausreichend Zeit, aber verhindert unendliches Warten.
    const unsigned long timeout_ms = 50;

    while (digitalRead(_doutPin) == HIGH);
    // Verzögerung nach der Fallflanke von DOUT, bevor der erste PD_SCK-Puls kommt (T1 > 1us)
    delayMicroseconds(1);


    // Lesen der 24 Datenbits. Der HX71708 gibt die Daten MSB (Most Significant Bit) zuerst aus
    for (i = 0; i < 24; i++) {
        digitalWrite(_pdSckPin, HIGH);
        delayMicroseconds(1);      // High-Zeit T3 < 50us 
        digitalWrite(_pdSckPin, LOW);  // PD_SCK Low-Pegel 
        delayMicroseconds(1);            // Low-Zeit T4 > 0.2us 
        bcd = bcd << 1; // Schiebe den bisher gelesenen Wert nach links, um Platz für das nächste Bit zu schaffen
        if (digitalRead(_doutPin) == HIGH) {
            bcd++; // Wenn DOUT High ist, setze das aktuelle Bit auf 1
        }
    }

    // Senden der zusätzlichen Taktimpulse zur Auswahl der nächsten Ausgangsdatenrate 
    // Für 320Hz sind insgesamt 28 Taktimpulse erforderlich (24 Datenpulse + 4 zusätzliche Pulse) 
    for (i = 0; i < 4; i++) { // Loop für N=4 zusätzliche Taktimpulse
        digitalWrite(_pdSckPin, HIGH);
        _custom_nop_delay();
        digitalWrite(_pdSckPin, LOW);
        _custom_nop_delay();
    }

    //Umwandlung des 24-Bit Wertes in einen vorzeichenlosen Wert.
    bcd = bcd ^ 0x800000;

    return bcd;
}


    /**
     * @brief Liest alle 4 Sensoren und gibt die Werte als Array zurück.
     * Bisher nicht implementiert.
     * @return Ein Array mit den 24-Bit Werten aller 4 Sensoren.
     */
    long read320Hz_All(void){

    }

    /**
     * @brief Ermittelt den Nullpunkt des Sensors.
     */
    void HX71708_ADC::tare() {
        long sum = 0;
        for (int i = 0; i < 20; i++) {
            sum += read320Hz(); // Führe 2 Messungen durch, um den Nullpunkt zu ermitteln
            delay(10); // Kurze Pause zwischen den Messungen
        }
        _offset = sum / 20; // Berechne den Durchschnitt der Messungen als Offset
}
/**
 * @brief Gibt Sensorwerte zurück, die um den Nullpunkt korrigiert sind.
 * @return Der um den Nullpunkt korrigierte 24-Bit Wert.
 */
    float HX71708_ADC::read_corrected() {
        long rawValue = read320Hz(); // Lese den Rohwert vom ADC

        return _scale_factor * float(rawValue - _offset); // Korrigiere den Wert um den Offset
}

/**
 * @brief Kalibriert den Sensor mit einem bekannten Gewicht.
 *
 * @param knownWeightGrams Das bekannte Gewicht in Gramm, das auf den Sensor gelegt wurde.
 */
    void HX71708_ADC::calibrate(int knownWeightGrams) {
    if (knownWeightGrams <= 0) {
        Serial.println("Fehler: Bekanntes Gewicht muss größer als 0 sein.");
        return;
    }

    // Lese den Rohwert mit dem bekannten Gewicht
    long rawValueWithWeight = 0;
    int numReadings = 20;
    Serial.print("Kalibrierung wird durchgeführt mit ");
    Serial.print(knownWeightGrams);
    Serial.println(" Gramm. Bitte Gewicht auflegen.");

    // Warte, bis der Benutzer das Gewicht aufgelegt hat (optional)
    delay(10000); // 5 Sekunden warten

    for (int i = 0; i < numReadings; i++) {
        rawValueWithWeight += read320Hz();
        delay(10);
    }
    rawValueWithWeight /= numReadings;

    // Berechne den Skalierungsfaktor
    // Skalierungsfaktor = (Bekanntes Gewicht) / (Rohwert mit Gewicht - Nullpunkt)
    _scale_factor = knownWeightGrams / (float)(rawValueWithWeight - _offset);
    Serial.print("Kalibrierungsfaktor gesetzt auf: ");
    Serial.println(_scale_factor, 6); // Ausgabe mit 6 Dezimalstellen
}
