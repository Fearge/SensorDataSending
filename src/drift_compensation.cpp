#include "drift_compensation.h"

void initialize_drift_trackers(DriftTracker trackers[], uint8_t num_sensors) {
    unsigned long now = millis();
    for (uint8_t i = 0; i < num_sensors; i++) {
        trackers[i].last_activity_time = now;
        trackers[i].idle = false;
        trackers[i].last_update_time = now;
    }
    Serial.println("INFO: Drift-Tracker initialisiert");
}

void update_drift_compensation(
    HX71708_ADC sensors[],
    DriftTracker trackers[],
    uint8_t num_sensors,
    long balance,
    long presence_threshold
) {
    unsigned long now = millis();

    // Prüfe Balance um zu bestimmen ob System generell idle ist
    // (Wenn balance == 0, dann sind vermutlich alle Sensoren ohne Last)
    bool system_idle = (balance == 0);

    // Für jeden Sensor: Track Idle-Zeit und triggere Drift-Korrektur
    for (uint8_t i = 0; i < num_sensors; i++) {
        // Update Tracking nur einmal pro DRIFT_UPDATE_INTERVAL_MS
        if ((now - trackers[i].last_update_time) < AppConfig::DRIFT_UPDATE_INTERVAL_MS) {
            continue;
        }
        trackers[i].last_update_time = now;

        // Wenn System idle ist: Inkrement Idle-Zähler
        if (system_idle) {
            unsigned long idle_duration = now - trackers[i].last_activity_time;

            // State-Transition: idle wird wahr wenn DRIFT_IDLE_TIME_MS überschritten
            if (!trackers[i].idle && idle_duration > AppConfig::DRIFT_IDLE_TIME_MS) {
                trackers[i].idle = true;
                Serial.print("INFO: Sensor ");
                Serial.print(i);
                Serial.println(" ist jetzt idle (Drift-Korrektur aktiv)");
            }

            // Wenn idle UND genug Zeit seit letzter Korrektur vergangen:
            // Triggere sanfte Offset-Anpassung
            if (trackers[i].idle && idle_duration > AppConfig::DRIFT_IDLE_TIME_MS) {
                // Cooldown prüfen: nur alle DRIFT_COOLDOWN_MS versuchen
                static unsigned long last_tare_time[4] = {0, 0, 0, 0};
                if ((now - last_tare_time[i]) > AppConfig::DRIFT_COOLDOWN_MS) {
                    Serial.print("INFO: Triggering soft_tare_update für Sensor ");
                    Serial.println(i);
                    sensors[i].soft_tare_update(AppConfig::DRIFT_WEIGHT_FACTOR);
                    last_tare_time[i] = now;
                }
            }
        } else {
            // System aktiv (balance != 0): Reset Idle-Status
            if (trackers[i].idle) {
                trackers[i].idle = false;
                Serial.print("INFO: Sensor ");
                Serial.print(i);
                Serial.println(" ist nicht mehr idle");
            }
            trackers[i].last_activity_time = now;
        }
    }
}
