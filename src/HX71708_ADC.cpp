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
    _timeout_active = false;
    _last_read_timed_out = false;
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
    delayMicroseconds(200); // Eine Verzögerung von 200us ist größer als die geforderten 100us
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
    const unsigned long timeout_ms = 50;
    _last_read_timed_out = false;

    // Stelle sicher, dass PD_SCK zunächst LOW ist, bevor auf DOUT gewartet wird
    digitalWrite(_pdSckPin, LOW);

    // Warten, bis DOUT auf Low geht. Dies signalisiert, dass der A/D-Wandler bereit ist, Daten auszugeben
    // Wenn DOUT nicht Low geht, kann dies auf ein Problem hinweisen, und der ADC muss zurückgesetzt werden
    unsigned long startTime = millis();

    while (digitalRead(_doutPin) == HIGH) {
        if (millis() - startTime > timeout_ms) {
            if (!_timeout_active) {
                Serial.print("WARN: HX71708 Timeout an DOUT-Pin ");
                Serial.println(_doutPin);
            }
            _timeout_active = true;
            _last_read_timed_out = true;

            // Kurzer Reset-Puls fuer den ADC zur Erholung nach Bus-/Sensorhaenger.
            digitalWrite(_pdSckPin, HIGH);
            delayMicroseconds(150);
            digitalWrite(_pdSckPin, LOW);

            // Sicherer Fallback: liefert nach read_corrected() einen Wert nahe 0.
            return _offset;
        }
        yield();
    }

    if (_timeout_active) {
        Serial.print("INFO: HX71708 wieder erreichbar an DOUT-Pin ");
        Serial.println(_doutPin);
        _timeout_active = false;
    }

    // Verzögerung nach der Fallflanke von DOUT, bevor der erste PD_SCK-Puls kommt (T1 > 1us)
    delayMicroseconds(1);


    // Lesen der 24 Datenbits. Der HX71708 gibt MSB (Most Significant Bit) zuerst aus
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
 * @brief Ermittelt den Nullpunkt des Sensors.
 */
void HX71708_ADC::tare() {
    constexpr int block_count = 8;
    constexpr int block_size = 40;
    constexpr int max_attempts_per_block = block_size * 6;
    long block_means[block_count] = {0};
    int valid_block_count = 0;

    auto collect_block_mean = [&](long &out_mean, int &out_valid_samples) -> bool {
        long block_sum = 0;
        int valid_samples = 0;
        int attempts = 0;

        while (valid_samples < block_size && attempts < max_attempts_per_block) {
            long sample = read320Hz();
            attempts++;

            if (_last_read_timed_out) {
                delay(2);
                continue;
            }

            block_sum += sample;
            valid_samples++;
            delay(10);
        }

        out_valid_samples = valid_samples;
        if (valid_samples == block_size) {
            out_mean = block_sum / block_size;
            return true;
        }
        return false;
    };

    auto sort_ascending = [&](long values[], int count) {
        for (int i = 1; i < count; i++) {
            long current_value = values[i];
            int position = i - 1;
            while (position >= 0 && values[position] > current_value) {
                values[position + 1] = values[position];
                position--;
            }
            values[position + 1] = current_value;
        }
    };

    for (int block = 0; block < block_count; block++) {
        long block_mean = 0;
        int valid_samples = 0;
        if (collect_block_mean(block_mean, valid_samples)) {
            block_means[valid_block_count] = block_mean;
            valid_block_count++;
        } else {
            Serial.print("WARN: Tare-Block verworfen (Sensor DOUT ");
            Serial.print(_doutPin);
            Serial.print(") valide Samples: ");
            Serial.print(valid_samples);
            Serial.print("/");
            Serial.println(block_size);
        }
    }

    if (valid_block_count == 0) {
        Serial.print("WARN: Tare abgebrochen, keine validen Samples an DOUT ");
        Serial.println(_doutPin);
        return;
    }

    sort_ascending(block_means, valid_block_count);

    int trim_per_side = (valid_block_count >= 5) ? 1 : 0;
    int start_index = trim_per_side;
    int end_index = valid_block_count - trim_per_side;

    long trimmed_sum = 0;
    for (int i = start_index; i < end_index; i++) {
        trimmed_sum += block_means[i];
    }

    int used_blocks = end_index - start_index;
    if (used_blocks <= 0) {
        Serial.print("WARN: Tare abgebrochen, keine gueltigen Bloecke nach Trim an DOUT ");
        Serial.println(_doutPin);
        return;
    }

    _offset = trimmed_sum / used_blocks;
}

/**
 * @brief Gibt Sensorwerte zurück, die um den Nullpunkt korrigiert sind.
 * @return Der um den Nullpunkt korrigierte 24-Bit Wert.
 */
float HX71708_ADC::read_corrected() {
    long rawValue = read320Hz(); // Lese den Rohwert vom ADC

    return _scale_factor * long(rawValue - _offset); // Korrigiere den Wert um den Offset
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


bool HX71708_ADC::is_idle() const {
    return _idle;
}

void HX71708_ADC::mark_activity() {
    _idle = false;
    _last_activity_time = millis();
}

void HX71708_ADC::update_drift_compensation(long balance, long presence_threshold) {
    unsigned long now = millis();
    
    // Prüfe ob System generell idle ist
    bool system_idle = (balance == 0);
    
    // Update-Rate begrenzen
    if ((now - _last_update_time) < DRIFT_UPDATE_INTERVAL_MS) {
        return;
    }
    _last_update_time = now;
    
    if (system_idle) {
        unsigned long idle_duration = now - _last_activity_time;
        
        // Transition zu idle bei Überschreitung von DRIFT_IDLE_TIME_MS
        if (!_idle && idle_duration > DRIFT_IDLE_TIME_MS) {
            _idle = true;
        }
        
        // Wenn idle UND Cooldown abgelaufen: Trigger sanfte Korrektur
        if (_idle && idle_duration > DRIFT_IDLE_TIME_MS) {
            if ((now - _last_tare_time) > DRIFT_COOLDOWN_MS) {
                soft_tare_update(DRIFT_WEIGHT_FACTOR);
                _last_tare_time = now;
            }
        }
    } else {
        // System aktiv: Reset idle state
        if (_idle) {
            _idle = false;
        }
        _last_activity_time = now;
    }
}

/**
 * @brief Sanfte Drift-Kompensation durch exponentiell gewichtete Offset-Anpassung.
 *        Wird aufgerufen, wenn der Sensor längere Zeit im Idle-Zustand war.
 *        Die neue Offset wird nicht sofort ersetzt, sondern mit exponentieller Gewichtung
 *        kombiniert: offset_neu = offset_alt * (1 - weight) + neue_messung * weight
 * 
 * @param weight_factor Gewichtung des neuen Messwerts (0.0-1.0). Z.B. 0.2 = 20% neu, 80% alt.
 */
void HX71708_ADC::soft_tare_update(float weight_factor) {
    if (weight_factor <= 0.0f || weight_factor > 1.0f) {
        Serial.println("WARN: soft_tare_update() weight_factor muss zwischen 0.0 und 1.0 sein");
        return;
    }

    // Sammle kurze Stichprobe (schnell, kein vollständiges Tare-Protokoll)
    long sample_sum = 0;
    int sample_count = 0;
    const int num_samples = 8;
    const int timeout_ms = 50;

    for (int i = 0; i < num_samples; i++) {
        unsigned long start = millis();
        // Timeout-Check: Falls Sensor hängt, abbrechen
        while (digitalRead(_doutPin) == HIGH && (millis() - start) < timeout_ms) {
            yield();
        }
        if ((millis() - start) >= timeout_ms) {
            Serial.print("WARN: soft_tare_update() Timeout an DOUT-Pin ");
            Serial.println(_doutPin);
            return; // Abbrechen bei Timeout
        }

        long raw = read320Hz();
        if (!_last_read_timed_out) {
            sample_sum += raw;
            sample_count++;
        }
        delay(5); // Kleine Pause zwischen Messungen
    }

    if (sample_count < 3) {
        Serial.print("WARN: soft_tare_update() unzureichend valide Samples an DOUT-Pin ");
        Serial.print(_doutPin);
        Serial.print(" (");
        Serial.print(sample_count);
        Serial.println("/");
        Serial.println(num_samples);
        return;
    }

    long new_sample = sample_sum / sample_count;

    // Exponentiell gewichtete Kombination
    float old_factor = 1.0f - weight_factor;
    long new_offset = (long)((_offset * old_factor) + (new_sample * weight_factor));

    // Nur loggen wenn Änderung größer als Rauschen ist
    if (abs(new_offset - _offset) > 100) {
        Serial.print("INFO: soft_tare_update() Drift-Korrektur an DOUT-Pin ");
        Serial.print(_doutPin);
        Serial.print(": ");
        Serial.print(_offset);
        Serial.print(" -> ");
        Serial.println(new_offset);
    }

    _offset = new_offset;
}

