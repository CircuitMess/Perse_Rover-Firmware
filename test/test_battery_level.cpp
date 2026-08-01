/**
 * test_battery_level.cpp
 *
 * Tests Rover battery level transitions using Hysteresis with the Rover's
 * specific thresholds: {0, 4, 15, 30, 70, 100}, margin=3.
 *
 * Levels:
 *   Critical = 0  [0, 4]
 *   VeryLow  = 1  [4, 15]
 *   Low      = 2  [15, 30]
 *   Mid      = 3  [30, 70]
 *   Full     = 4  [70, 100]
 *
 * Validates: Requirements 20.1, 20.2, 20.3
 *
 * Compile:
 *   g++ -std=c++17 -I../../Perse-Common/test -o test_battery_level test_battery_level.cpp && ./test_battery_level
 */

#include "freertos_mock.h"
#include <vector>
#include <initializer_list>

// ============ Inline Hysteresis class (from Perse-Common/src/Hysteresis.cpp) ============

class Hysteresis {
public:
    Hysteresis(std::initializer_list<int> thresholds, int margin)
        : Thresholds(thresholds), LevelCount(thresholds.size() - 1), Margin(margin), currentLevel(0) {}

    int get() const {
        if (currentLevel < 0) return 0;
        else if (currentLevel >= LevelCount) return LevelCount - 1;
        return currentLevel;
    }

    int update(int val) {
        int lb = Thresholds[currentLevel];
        if (currentLevel > 0) lb -= Margin;

        int ub = Thresholds[currentLevel + 1];
        if (currentLevel < LevelCount) ub += Margin;

        if (val < lb || val > ub) {
            currentLevel = findLevel(val);
        }

        return get();
    }

    int reset(int val = 0) {
        currentLevel = findLevel(val);
        return get();
    }

private:
    const std::vector<int> Thresholds;
    const int LevelCount;
    const int Margin;
    int currentLevel;

    int findLevel(int val) {
        int i;
        for (i = 0; i < LevelCount; i++) {
            if (val >= Thresholds[i] && val <= Thresholds[i + 1]) break;
        }
        return i;
    }
};

// ============ Mock ADCReader ============
// Simulates an ADC that returns preset percentage values.

class MockADCReader {
public:
    void setPercentage(int perc) { value = perc; }
    int getValue() const { return value; }
private:
    int value = 50; // default: mid-range
};

// ============ Simulated Battery Logic ============
// Extracts the core battery sampling logic from Battery::sample():
// - Updates hysteresis with the ADC percentage value
// - Sets shutdown flag when level transitions to Critical (0)

enum Level { Critical = 0, VeryLow, Low, Mid, Full, COUNT };

struct BatterySimulator {
    Hysteresis hysteresis;
    MockADCReader adc;
    bool shutdown = false;

    BatterySimulator()
        : hysteresis({0, 4, 15, 30, 70, 100}, 3) {}

    Level getLevel() const {
        return (Level)hysteresis.get();
    }

    void sample() {
        if (shutdown) return;

        hysteresis.update(adc.getValue());

        if (getLevel() == Critical) {
            shutdown = true;
        }
    }

    void setADC(int perc) {
        adc.setPercentage(perc);
    }

    void reset(int perc) {
        shutdown = false;
        hysteresis.reset(perc);
    }
};

// ============ Tests ============

/**
 * Requirement 20.1: WHEN the battery ADC reading transitions from above 4% to
 * below 4%, THE Battery level SHALL change to Critical (level 0) after
 * accounting for hysteresis margin.
 */
void test_transition_above_4_to_below_4_triggers_critical() {
    BatterySimulator bat;

    // Start at 10% → Level 1 (VeryLow: [4, 15])
    bat.setADC(10);
    bat.sample();
    ASSERT(bat.getLevel() == VeryLow);

    // Move to 5% — still within VeryLow level [4, 15]
    // Lower bound for level 1 is Thresholds[1] - Margin = 4 - 3 = 1
    bat.setADC(5);
    bat.sample();
    ASSERT(bat.getLevel() == VeryLow);

    // Move to 2% — still within hysteresis margin (lb = 4-3 = 1)
    bat.setADC(2);
    bat.sample();
    ASSERT(bat.getLevel() == VeryLow);

    // Move to 0% — below the lower bound (1), triggers level change to Critical
    bat.setADC(0);
    bat.sample();
    ASSERT(bat.getLevel() == Critical);
}

/**
 * Requirement 20.2: WHEN the battery level is Critical, THE Battery SHALL set
 * the shutdown flag to true.
 */
void test_critical_level_sets_shutdown_flag() {
    BatterySimulator bat;

    // Start at a mid level, no shutdown
    bat.setADC(50);
    bat.sample();
    ASSERT(bat.shutdown == false);
    ASSERT(bat.getLevel() == Mid);

    // Jump directly to 0% — must trigger Critical
    bat.setADC(0);
    bat.sample();
    ASSERT(bat.getLevel() == Critical);
    ASSERT(bat.shutdown == true);
}

/**
 * Additional test: verify that once shutdown is set, further samples are ignored.
 */
void test_shutdown_prevents_further_sampling() {
    BatterySimulator bat;

    // Get to Critical and shutdown
    bat.setADC(0);
    bat.sample();
    ASSERT(bat.shutdown == true);
    ASSERT(bat.getLevel() == Critical);

    // Even if ADC recovers, sample() does nothing because shutdown == true
    bat.setADC(80);
    bat.sample();
    // Level stays Critical because sample() returned early
    ASSERT(bat.getLevel() == Critical);
    ASSERT(bat.shutdown == true);
}

/**
 * Requirement 20.3: WHEN the battery oscillates at exactly the Critical
 * threshold (4%) ± margin(3), THE hysteresis SHALL prevent rapid level toggling.
 *
 * For level 1 (VeryLow), lower bound = Thresholds[1] - Margin = 4 - 3 = 1.
 * Values between 1 and 4 should NOT cause a transition away from VeryLow.
 */
void test_oscillation_at_threshold_does_not_toggle() {
    BatterySimulator bat;

    // Start at 10% → VeryLow (level 1)
    bat.setADC(10);
    bat.sample();
    ASSERT(bat.getLevel() == VeryLow);

    // Oscillate around the critical threshold boundary: values 1–7
    // All these are within the hysteresis band for VeryLow
    // (lb=1, ub=15+3=18 for level 1)
    int oscillating_values[] = {4, 2, 5, 3, 4, 1, 4, 3, 2, 4, 5, 3, 1};
    for (int v : oscillating_values) {
        bat.setADC(v);
        bat.sample();
        // Should stay VeryLow due to hysteresis — none go below lb=1
        ASSERT(bat.getLevel() == VeryLow);
        ASSERT(bat.shutdown == false);
    }
}

/**
 * Test that values oscillating between exactly 1 and 7 (at the margin boundary)
 * do not cause rapid toggling between Critical and VeryLow.
 */
void test_no_rapid_toggling_at_margin_boundary() {
    BatterySimulator bat;

    // Start at 10% → use reset to place us firmly in VeryLow (level 1)
    bat.reset(10);
    ASSERT(bat.getLevel() == VeryLow);

    int toggle_count = 0;
    Level prev = bat.getLevel();

    // Rapidly alternate between 1 and 7 (both within VeryLow's hysteresis band)
    // VeryLow: lb = Thresholds[1] - Margin = 4 - 3 = 1, ub = Thresholds[2] + Margin = 15 + 3 = 18
    // So values 1 through 18 should all stay within VeryLow
    for (int i = 0; i < 20; i++) {
        int val = (i % 2 == 0) ? 1 : 7;
        bat.setADC(val);
        bat.sample();
        Level cur = bat.getLevel();
        if (cur != prev) toggle_count++;
        prev = cur;
    }

    // Hysteresis should prevent toggling: VeryLow lb=1, so values >= 1 stay VeryLow
    ASSERT(toggle_count == 0);
    ASSERT(bat.shutdown == false);
}

/**
 * Test direct transition from Full to Critical (large drop).
 */
void test_large_drop_to_critical() {
    BatterySimulator bat;

    // Start at 80% → Full (level 4)
    bat.setADC(80);
    bat.sample();
    ASSERT(bat.getLevel() == Full);
    ASSERT(bat.shutdown == false);

    // Sudden drop to 0%
    bat.setADC(0);
    bat.sample();
    ASSERT(bat.getLevel() == Critical);
    ASSERT(bat.shutdown == true);
}

// ============ Main ============

int main() {
    printf("=== test_battery_level ===\n");

    RUN_TEST(test_transition_above_4_to_below_4_triggers_critical);
    RUN_TEST(test_critical_level_sets_shutdown_flag);
    RUN_TEST(test_shutdown_prevents_further_sampling);
    RUN_TEST(test_oscillation_at_threshold_does_not_toggle);
    RUN_TEST(test_no_rapid_toggling_at_margin_boundary);
    RUN_TEST(test_large_drop_to_critical);

    TEST_SUMMARY();
}
