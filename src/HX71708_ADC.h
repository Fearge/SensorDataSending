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
    // NOTE: scale factor removed; system will work with raw counts (offset-corrected)
    /**
     * @brief Konstruktor für die HX71708_ADC-Klasse.
     * @param pdSckPin Pin, der mit dem PD_SCK-Pin des HX71708 verbunden ist.
     * @param doutPin Pin, der mit dem DOUT-Pin des HX71708 verbunden ist.
     *                
     */
    HX71708_ADC(int pdSckPin, int doutPin);

    /**
     * @brief Initialisiert den HX71708 ADC nach dem Einschalten.
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
    // scale factor removed; no set_scale_factor/get_scale_factor
    /**
     * @brief Gibt den rohen, um Offset korrigierten Sensorwert zurück (counts).
     * @return Offset-korrigierter Rohwert als `long`.
     */
    long read_corrected(void);
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

     /* Note: `calibrate` and scaling were removed — system uses raw counts.
         Use `tare()` to set zero offset if needed. */

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
     * @brief Prüft den aktuellen Sensorwert und aktualisiert Drift-Kompensation sowie Idle-Status.
     *        Sollte regelmäßig (ca. 1x pro Sekunde) aufgerufen werden.
     * @param sensor_value Offset-korrigierter Sensorwert in Counts.
     * @param presence_threshold Schwelle, ab der der Sensor als belastet gilt.
     * @return void
     */
    void update_drift_compensation(long sensor_value, long presence_threshold);
};

#endif // HX71708_ADC_H