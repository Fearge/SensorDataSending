#ifndef HX71708_ADC_H
#define HX71708_ADC_H

#include <Arduino.h> // Für Arduino-Funktionen wie pinMode, digitalWrite, digitalRead, delayMicroseconds, millis, Serial

/**
 * @brief Eine Klasse zur Steuerung und zum Auslesen des HX71708 24-Bit A/D-Wandlers.
 *
 * Der HX71708 ist ein hochpräziser 24-Bit A/D-Wandler, der speziell für
 * elektronische Waagen entwickelt wurde [1]. Er zeichnet sich durch hohe
 * Integration, schnelle Reaktionszeiten und starke Anti-Interferenz-Fähigkeit aus [1].
 * Der Chip benötigt lediglich 2 I/O-Pins eines Mikrocontrollers zur Steuerung,
 * einschließlich der Abschaltsteuerung (Power-Down-Control) [1].
 */
class HX71708_ADC {
private:
    /**
     * @brief Eine private Hilfsfunktion für kurze Verzögerungen, die _nop_() ersetzt.
     *        Entspricht etwa einer Verzögerung von 1 Mikrosekunde.
     *        Dies ist notwendig, um die Timing-Anforderungen des HX71708 Datenblatts
     */
    void _custom_nop_delay(void) {
        ((void)0);
    }

public:
    int _pdSckPin; // Pin für PD_SCK (Power Down Control und Serieller Takt) [2, 3]
    int _doutPin;  // Pin für DOUT (Serielle Datenausgabe) [2, 3]
    long _offset;
    float _scale_factor; // Skalierungsfaktor für die Kalibrierung des Sensors
    /**
     * @brief Konstruktor für die HX71708_ADC-Klasse.
     * @param pdSckPin Der GPIO-Pin, der mit dem PD_SCK-Pin des HX71708 verbunden ist.
     *                 PD_SCK ist ein digitaler Eingang für die Abschaltsteuerung
     *                 (High-Level ist aktiv) und den seriellen Takteingang [3, 5].
     * @param doutPin Der GPIO-Pin, der mit dem DOUT-Pin des HX71708 verbunden ist.
     *                DOUT ist ein digitaler Ausgang für die serielle Datenausgabe [3, 5].
     */
    HX71708_ADC(int pdSckPin, int doutPin);

    /**
     * @brief Initialisiert den HX71708 ADC nach dem Einschalten.
     *        Konfiguriert die GPIO-Pins und setzt den ADC in einen bekannten Zustand.
     *        Diese Methode sollte einmalig für jede Instanz beim Start des Systems aufgerufen werden.
     *
     *        Laut den Hinweisen zur Nutzung soll der Mikrocontroller den PD_SCK-Pin
     *        beim Einschalten des ADC-Chips für mehr als 100 Mikrosekunden auf HIGH ziehen
     *        und dann wieder auf LOW setzen, um den ADC-Chip zurückzusetzen [6].
     */
    void begin(void);


    /**
     * @brief Gibt den Offset des Sensors zurück. 
     */
    long get_offset(void);

    /**
     * @brief Setzt den Offset des Sensors.
     * @param offset Der neue Offset-Wert, der gesetzt werden soll.
     */
    void set_offset(long offset);
    /**
     * @brief Setzt den Skalierungsfaktor des Sensors.
     * @param scale_factor Der neue Faktor, der gesetzt werden soll.
     */
    void set_scale_factor(float scale_factor);

    /**
     * @brief Gibt den Skalierungsfaktor des Sensors zurück.
     * @return Der aktuelle Skalierungsfaktor.
     */
    float get_scale_factor(void);

    /**
     * @brief Wandelt den Rohwert des Sensors in Gramm um.
     * @param raw Der Rohwert des Sensors.
     * @return Der um den Offset korrigierte Wert in Gramm.
     */
    float toGrams(long raw);
    /**
     * @brief Liest einen 24-Bit Sensorwert vom HX71708 ADC und setzt die nächste Datenrate auf 320Hz.
     *        Die serielle Kommunikation erfolgt über die Pins PD_SCK und DOUT [3].
     *        Die 24-Bit-Ausgangsdaten sind im binären Zweierkomplement-Code formatiert,
     *        wobei das MSB das Vorzeichenbit ist [7].
     *        Die Umwandlung in einen vorzeichenlosen Wert erfolgt durch XOR mit 0x800000 [4].
     * @return Der umgewandelte 24-Bit Wert. Gibt 0 zurück, falls ein Timeout auftritt
     *         und der ADC zurückgesetzt werden muss.
     *         Nach einem Reset oder einer Änderung der Datenrate benötigt der ADC
     *         vier Datenzyklen, um stabile Ausgangsdaten zu liefern [8, 9].
     */
    long read320Hz(void);

    /**
     * @brief ermittelt den Nullpunkt des Sensors.
     * @return void
     */
    void tare(void);

    /**
     * @brief Gibt Sensor Werte zurück, die um den Nullpunkt korrigiert sind.
     * @return Der um den Nullpunkt korrigierte 24-Bit Wert.
     */
    float read_corrected(void);

    /**
     * @brief berechnet den Skalierungsfaktor des Sensors und setzt diesen als Parameter
     * @param known_weight Ein bekanntes Gewicht, zur ermittelung des Skalierungsfaktors
     * @return void
     */
    void calibrate(int known_weight);
};

#endif // HX71708_ADC_H