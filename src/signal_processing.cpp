#include "signal_processing.h"

long calculate_balance(long pair_0, long pair_1, long presence_threshold, long balance_max) {
    long pair_0_abs = abs(pair_0);
    long pair_1_abs = abs(pair_1);
    long total_load = pair_0_abs + pair_1_abs;

    if (total_load <= presence_threshold) {
        return 0;
    }

    long balance = ((pair_1_abs - pair_0_abs) * balance_max) / total_load;
    if (balance < -balance_max) {
        balance = -balance_max;
    }
    if (balance > balance_max) {
        balance = balance_max;
    }

    return balance;
}

long compute_balance_from_sensors(
    HX71708_ADC sensors[],
    uint8_t num_sensors,
    long presence_threshold,
    long balance_max
) {
    long all_values[4] = {0};
    uint8_t read_count = (num_sensors > 4) ? 4 : num_sensors;

    for (uint8_t i = 0; i < read_count; i++) {
        all_values[i] = sensors[i].read_corrected();
        sensors[i].update_drift_compensation(all_values[i], presence_threshold);
    }

    long pair_0 = (all_values[0] + all_values[1]) / 2;
    long pair_1 = (all_values[2] + all_values[3]) / 2;
    return calculate_balance(pair_0, pair_1, presence_threshold, balance_max);
}
