#ifndef HX71708_ADC_H
#define HX71708_ADC_H

#include <Arduino.h> // Für Arduino-Funktionen wie pinMode, digitalWrite, digitalRead, delayMicroseconds, millis, Serial

/**
 * @brief Eine Klasse zur Steuerung und zum Auslesen des HX71708 24-Bit A/D-Wandlers.
 */
class HX71708_ADC {
private:
    bool _timeout_active;
    bool _last_read_timed_out;
    
    // Drift-Kompensations-State
    unsigned long _last_activity_time;
    unsigned long _last_update_time;
    unsigned long _last_tare_time;
    bool _idle;

    void _custom_nop_delay(void) {
        ((void)0);
    }
    
    // Drift-Kompensations-Konstanten (sollten mit app_config synchron sein)
    static constexpr unsigned long DRIFT_IDLE_TIME_MS = 30000;      // 30s
    static constexpr float DRIFT_WEIGHT_FACTOR = 0.2f;               // 20%
    static constexpr unsigned long DRIFT_COOLDOWN_MS = 5000;         // 5s
    static constexpr unsigned long DRIFT_UPDATE_INTERVAL_MS = 1000;  // 1s

public:
    int _pdSckPin; // Pin für PD_SCK (Power Down Control und Serieller Takt)
    int _doutPin;  // Pin für DOUT (Serielle Datenausgabe)
    long _offset;
    float _scale_factor; // Skalierungsfaktor für die Kalibrierung des Sensors
    /**
     * @brief Konstruktor für die HX71708_ADC-Klasse.
     * @param pdSckPin Der GPIO-Pin, der mit dem PD_SCK-Pin des HX71708 verbunden ist.
     *                 PD_SCK ist ein digitaler Eingang für die Abschaltsteuerung
     *                 (High-Level ist aktiv) und den seriellen Takteingang.
     * @param doutPin Der GPIO-Pin, der mit dem DOUT-Pin des HX71708 verbunden ist.
     *                DOUT ist ein digitaler Ausgang für die serielle Datenausgabe.
     */
    HX71708_ADC(int pdSckPin, int doutPin);

    /**
     * @brief Initialisiert den HX71708 ADC nach dem Einschalten.
     *        Konfiguriert die GPIO-Pins und setzt den ADC in einen bekannten Zustand.
     *        Diese Methode sollte einmalig für jede Instanz beim Start des Systems aufgerufen werden.
     *
     *        Laut den Hinweisen zur Nutzung soll der Mikrocontroller den PD_SCK-Pin
     *        beim Einschalten des ADC-Chips für mehr als 100 Mikrosekunden auf HIGH ziehen
     *        und dann wieder auf LOW setzen, um den ADC-Chip zurückzusetzen.
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
     *        Die serielle Kommunikation erfolgt über die Pins PD_SCK und DOUT.
     *        Die 24-Bit-Ausgangsdaten sind im binären Zweierkomplement-Code formatiert,
     *        wobei das MSB das Vorzeichenbit ist [7].
     *        Die Umwandlung in einen vorzeichenlosen Wert erfolgt durch XOR mit 0x800000.
    * @return Der umgewandelte 24-Bit Wert. Bei Timeout wird ein sicherer Fallback
    *         zurückgegeben und der ADC kurz zurückgesetzt.
     *         Nach einem Reset oder einer Änderung der Datenrate benötigt der ADC
     *         vier Datenzyklen, um stabile Ausgangsdaten zu liefern.
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

    /**
     * @brief Sanfte Drift-Kompensation: Passt den Offset exponentiell gewichtet an.
     *        Wird aufgerufen wenn Sensor längere Zeit idle ist (keine Last).
     *        Neue Offsets werden nicht sofort ersetzt, sondern gewichtet kombiniert:
     *        offset_neu = offset_alt * (1 - weight) + neue_messung * weight
     * @param weight_factor Gewichtung für neuen Messwert (0.0 - 1.0), z.B. 0.2 = 20%
     * @return void
     */
    void soft_tare_update(float weight_factor);
    
    /**
     * @brief Gibt an, ob Sensor aktuell im Idle-Zustand ist (ohne Last, längere Zeit).
     * @return true wenn idle, false sonst
     */
    bool is_idle() const;
    
    /**
     * @brief Markiert den Sensor als aktiv (mit Last). Setzt activity-Timer zurück.
     * @return void
     */
    void mark_activity();
    
    /**
     * @brief Prüft Balance und aktualisiert Drift-Kompensation wenn nötig.
     *        Sollte regelmäßig (ca. 1x pro Sekunde) aufgerufen werden.
     * @param balance Aktuelle Balance-Messung (um zu prüfen ob System idle)
     * @param presence_threshold Schwelle, ab der Sensor als "belastet" gilt
     * @return void
     */
    void update_drift_compensation(long balance, long presence_threshold);
};

#endif // HX71708_ADC_H