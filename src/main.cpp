#include <Arduino.h>
#include <HX71708_ADC.h> // Die HX71708 Bibliothek für den 24-Bit A/D-Wandler
#include "osc.h" // Die OSC Bibliothek für die Kommunikation
#include "server.h" // Die Server-Bibliothek für WiFi und WebServer
// Pin-Definitionen 
const uint8_t SCK_PINS[] = {3, 5, 7, 9};
const uint8_t DOUT_PINS[] = {2, 4, 6, 8};
const uint8_t NUM_SENSORS = 4;

//Konstanten
const long ZERO_BORDER = 10000; //untere Grenze für Zero-Signal
const long NORMALIZED_MAX = 1024; // Obergrenze fuer normalisierte Ausgabe
const float SCALE_FACTORS[] = {0.0186, 0.0186, 0.0186, 0.0186};

bool max_value_ready = false;
long current_max_value = ZERO_BORDER + 1;

// Array von Sensor-Objekten
HX71708_ADC sensors[NUM_SENSORS] = {
    HX71708_ADC(SCK_PINS[0], DOUT_PINS[0]),
    HX71708_ADC(SCK_PINS[1], DOUT_PINS[1]),
    HX71708_ADC(SCK_PINS[2], DOUT_PINS[2]),
    HX71708_ADC(SCK_PINS[3], DOUT_PINS[3])
};

void initialize_sensors();
void calibrate_all_sensors();
void calibration_values();
long calc_new_max_val();
long average_of_array(long arr[], int size);
long normalize_value(long value, long min_in, long max_in, long max_out);

void setup() {
    Serial.begin(115200);
    delay(5000);
    Serial.println("Initialisiere HX71708 ADCs...");

    // Initialisierung aller Sensoren
    initialize_sensors();
    
    // Kalibrierung aller Sensoren
    calibrate_all_sensors();

    //calibration_values();
    
    Serial.println("Initialisierung abgeschlossen.");

    WiFiAP::initializeAP(); // Initialize WiFi in AP mode
    WiFiAP::printPort(); // Print the UDP port number
}

void loop() {
    long all_values[] = {0, 0, 0, 0};
    OSC::msg.empty(); // Leere die OSC-Nachricht vor dem Hinzufügen neuer Daten

    // Alle Sensoren lesen
    for (int i = 0; i < NUM_SENSORS; i++) {
        long value = sensors[i].read_corrected();
        all_values[i] = value;
    }

    long mean_values[2] = {(all_values[0] + all_values[1])/2, (all_values[2]+all_values[3])/2};
    long pair_0 = abs(mean_values[0]);
    long pair_1 = abs(mean_values[1]);
    long current_peak = (pair_0 + pair_1) / 2;

    if (current_peak <= ZERO_BORDER) {
        // Unterhalb der Schwelle wird fuer den naechsten Ueberschritt neu kalibriert.
        max_value_ready = false;
        OSC::msg.add((int32_t)0).add((int32_t)0);
    } else {
        if (!max_value_ready) {
            current_max_value = calc_new_max_val() + 5000; //etwas Puffer, da davor Mittelung stattfand
            max_value_ready = true;
        }
        if (current_peak > current_max_value) {
            current_max_value = current_peak;
        }
        long normalized_0 = normalize_value(pair_0, ZERO_BORDER, current_max_value, NORMALIZED_MAX);
        long normalized_1 = normalize_value(pair_1, ZERO_BORDER, current_max_value, NORMALIZED_MAX);
        OSC::msg.add((int32_t)normalized_0).add((int32_t)normalized_1);
    }

    // Sende die OSC-Nachricht
    OSC::sendMessage(OSC::msg);
    

    /*Serial.print((all_values[0] + all_values[1])/2);
    Serial.print(", ");
    Serial.print((all_values[2] + all_values[3])/2);
    Serial.println();*/
    

}

/**
 * @brief Berechnet Maximalwert basierend auf 1 Sekunde Messungen
 * 
 */
long calc_new_max_val() {
    // Berechne den neuen Maximalwert basierend auf den aktuellen Sensorwerten
    long readings[NUM_SENSORS] = {0, 0, 0, 0};
    long max_val = ZERO_BORDER + 1;

    for (int i = 0; i < 160; i++) {
        for (int j = 0; j < NUM_SENSORS; j++) {
            long value = sensors[j].read_corrected();
            readings[j] = abs(value);
        }

        long mean_abs = (long)(readings[0] + readings[1] + readings[2] + readings[3]) / 4;
        if (mean_abs > max_val) {
            max_val = mean_abs;
        }
    }

    return max_val;
}


/**
 * @brief Initialisiert alle Sensoren, indem die begin()-Methode für jede Instanz aufgerufen wird.
 *        Diese Methode sollte einmalig beim Start des Systems aufgerufen werden, um die Sensoren in einen bekannten Zustand zu versetzen.
 */

void initialize_sensors() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("Initialisiere Sensor ");
        Serial.println(i + 1);
        
        sensors[i].begin();
        delay(100);
    }
}

void calibrate_all_sensors() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("Kalibriere Sensor ");
        Serial.println(i + 1);
        
        sensors[i].tare();
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(" Tare abgeschlossen. Offset: ");
        Serial.println(sensors[i].get_offset());
        
        sensors[i].set_scale_factor(SCALE_FACTORS[i]);
        delay(1000);
    }
}

void calibration_values() {
    Serial.println("Kalibrierungswerte:");
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensors[i].calibrate(1245);
        Serial.print(", Skalierungsfaktor = ");
        Serial.println(sensors[i].get_scale_factor(), 6);
    }
}


long average_of_array(long arr[], int size) {
    long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum / size;
}

long normalize_value(long value, long min_in, long max_in, long max_out) {
    if (max_in <= min_in) {
        return 0;
    }

    long clamped = value;
    if (clamped < min_in) {
        clamped = min_in;
    }
    if (clamped > max_in) {
        clamped = max_in;
    }

    return map(clamped, min_in, max_in, 0, max_out);
}