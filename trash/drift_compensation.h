#ifndef DRIFT_COMPENSATION_H
#define DRIFT_COMPENSATION_H

#include <Arduino.h>
#include "HX71708_ADC.h"
#include "app_config.h"

/**
 * @brief Struktur für Idle-Tracking pro Sensor.
 *        Verfolgt, wie lange ein Sensor ohne Last "ruht" um Drift-Kompensation zu triggern.
 */
struct DriftTracker {
    unsigned long last_activity_time; // Zeitstempel der letzten nicht-Idle Messung
    bool idle;                        // Ist der Sensor aktuell im Idle-Zustand?
    unsigned long last_update_time;   // Zeitstempel des letzten Check-Durchgangs
};

/**
 * @brief Initialisiert alle Drift-Tracker für alle Sensoren.
 * @param trackers Array von DriftTracker Objekten (mindestens num_sensors groß)
 * @param num_sensors Anzahl der Sensoren
 */
void initialize_drift_trackers(DriftTracker trackers[], uint8_t num_sensors);

/**
 * @brief Prüft auf Idle-Bedingungen und triggert sanfte Drift-Korrektur bei Bedarf.
 *        Sollte regelmäßig aus der main loop aufgerufen werden (z.B. einmal pro Sekunde).
 *
 * @param sensors Array von HX71708_ADC Sensor-Objekten
 * @param trackers Array von DriftTracker Objekten (parallele Struktur zu sensors)
 * @param num_sensors Anzahl der Sensoren
 * @param balance Aktuell gemessene Balance (um zu prüfen ob idle)
 * @param presence_threshold Schwellenwert ab dem Sensor als "active" gilt
 */
void update_drift_compensation(
    HX71708_ADC sensors[],
    DriftTracker trackers[],
    uint8_t num_sensors,
    long balance,
    long presence_threshold
);

#endif // DRIFT_COMPENSATION_H
