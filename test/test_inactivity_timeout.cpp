/**
 * test_inactivity_timeout.cpp
 *
 * Tests the pure timeout calculation logic extracted from
 * InactivityService::calculateRemainingTicks().
 *
 * The logic:
 *   PairedTimeout   = 600000ms (10 minutes)
 *   UnpairedTimeout = 300000ms (5 minutes)
 *   remaining = (elapsed >= timeout) ? 0 : timeout - elapsed
 *   Activity resets the timer (timer = currentTime), so elapsed resets to 0.
 *
 * Validates: Requirements 22.1, 22.2, 22.3
 *
 * Compile:
 *   g++ -std=c++17 -I../../Perse-Common/test -o test_inactivity_timeout test_inactivity_timeout.cpp && ./test_inactivity_timeout
 */

#include "freertos_mock.h"
#include <cstdint>

// ============ Constants from InactivityService ============

static constexpr uint32_t UnpairedTimeout = 300000; // 5 mins in ms
static constexpr uint32_t PairedTimeout   = 600000; // 10 mins in ms

// ============ Extracted pure timeout calculation ============
// Mirrors InactivityService::calculateRemainingTicks() logic.
// Returns remaining time in ms (we test the arithmetic, not the tick conversion).

static uint32_t calculateRemainingMs(uint32_t timeout, uint32_t elapsed) {
    if (elapsed >= timeout) {
        return 0;
    }
    return timeout - elapsed;
}

// ============ Simulator for activity reset behavior ============
// Models how InactivityService tracks its timer and recalculates on activity.

struct InactivitySimulator {
    uint32_t timer = 0;       // last activity timestamp (millis)
    bool paired = false;

    uint32_t getTimeout() const {
        return paired ? PairedTimeout : UnpairedTimeout;
    }

    // Simulate calculateRemainingTicks given current simulated time
    uint32_t calculateRemainingMs(uint32_t currentTime) const {
        uint32_t elapsed = currentTime - timer;
        uint32_t timeout = getTimeout();
        if (elapsed >= timeout) {
            return 0;
        }
        return timeout - elapsed;
    }

    // Simulate activity reset (sets timer = currentTime)
    void notifyActivity(uint32_t currentTime) {
        timer = currentTime;
    }
};

// ============ Tests ============

/**
 * Requirement 22.1: WHEN the paired timeout is 600000ms and 300000ms have
 * elapsed, THE InactivityService SHALL sleep for exactly 300000ms.
 */
void test_paired_timeout_half_elapsed() {
    uint32_t remaining = calculateRemainingMs(PairedTimeout, 300000);
    ASSERT(remaining == 300000);
}

/**
 * Additional: paired timeout with 0 elapsed → full timeout remaining.
 */
void test_paired_timeout_zero_elapsed() {
    uint32_t remaining = calculateRemainingMs(PairedTimeout, 0);
    ASSERT(remaining == 600000);
}

/**
 * Additional: paired timeout with exactly 600000 elapsed → 0 remaining.
 */
void test_paired_timeout_exact_expiry() {
    uint32_t remaining = calculateRemainingMs(PairedTimeout, 600000);
    ASSERT(remaining == 0);
}

/**
 * Requirement 22.2: WHEN the unpaired timeout is 300000ms and 300001ms have
 * elapsed (beyond timeout), THE InactivityService SHALL use a wait time of 0
 * (immediate timeout check).
 */
void test_unpaired_timeout_exceeded_by_one() {
    uint32_t remaining = calculateRemainingMs(UnpairedTimeout, 300001);
    ASSERT(remaining == 0);
}

/**
 * Additional: unpaired timeout with exactly 300000 elapsed → 0 remaining.
 */
void test_unpaired_timeout_exact_expiry() {
    uint32_t remaining = calculateRemainingMs(UnpairedTimeout, 300000);
    ASSERT(remaining == 0);
}

/**
 * Additional: unpaired timeout with 299999 elapsed → 1ms remaining.
 */
void test_unpaired_timeout_one_ms_before_expiry() {
    uint32_t remaining = calculateRemainingMs(UnpairedTimeout, 299999);
    ASSERT(remaining == 1);
}

/**
 * Additional: unpaired timeout with large elapsed (well beyond timeout) → 0.
 */
void test_unpaired_timeout_large_elapsed() {
    uint32_t remaining = calculateRemainingMs(UnpairedTimeout, 1000000);
    ASSERT(remaining == 0);
}

/**
 * Requirement 22.3: WHEN activity resets the timer, THE InactivityService SHALL
 * recalculate the remaining time from the new timer value.
 *
 * Scenario: timer starts at 0, 200000ms elapse (200s into 300s unpaired timeout),
 * activity occurs resetting timer, then elapsed is recalculated from new timer.
 */
void test_activity_reset_recalculates_from_new_timer() {
    InactivitySimulator sim;
    sim.paired = false;
    sim.timer = 0;

    // At time=200000ms, 200000ms have elapsed → 100000ms remaining
    uint32_t remaining = sim.calculateRemainingMs(200000);
    ASSERT(remaining == 100000);

    // Activity occurs at time=200000ms → timer resets to 200000
    sim.notifyActivity(200000);

    // Now at time=200000ms, elapsed = 200000 - 200000 = 0 → full timeout remaining
    remaining = sim.calculateRemainingMs(200000);
    ASSERT(remaining == UnpairedTimeout); // 300000ms

    // At time=350000ms, elapsed = 350000 - 200000 = 150000 → 150000ms remaining
    remaining = sim.calculateRemainingMs(350000);
    ASSERT(remaining == 150000);
}

/**
 * Test activity reset with paired state change.
 * Timer resets when paired changes, new timeout value applies.
 */
void test_activity_reset_with_paired_state_change() {
    InactivitySimulator sim;
    sim.paired = false;
    sim.timer = 0;

    // At time=100000ms, unpaired: remaining = 300000 - 100000 = 200000
    uint32_t remaining = sim.calculateRemainingMs(100000);
    ASSERT(remaining == 200000);

    // Pairing occurs at time=100000ms (activity also resets timer)
    sim.paired = true;
    sim.notifyActivity(100000);

    // Now paired timeout is 600000ms, elapsed=0 → full 600000ms remaining
    remaining = sim.calculateRemainingMs(100000);
    ASSERT(remaining == PairedTimeout); // 600000ms

    // 300000ms later (time=400000ms), elapsed=300000 → 300000ms remaining
    remaining = sim.calculateRemainingMs(400000);
    ASSERT(remaining == 300000);
}

/**
 * Test multiple activity resets keep extending the deadline.
 */
void test_multiple_activity_resets_extend_deadline() {
    InactivitySimulator sim;
    sim.paired = false;
    sim.timer = 0;

    // Reset at time=100000
    sim.notifyActivity(100000);
    uint32_t remaining = sim.calculateRemainingMs(100000);
    ASSERT(remaining == 300000);

    // Reset at time=200000
    sim.notifyActivity(200000);
    remaining = sim.calculateRemainingMs(200000);
    ASSERT(remaining == 300000);

    // Reset at time=400000
    sim.notifyActivity(400000);
    remaining = sim.calculateRemainingMs(400000);
    ASSERT(remaining == 300000);

    // At time=600000 (200000ms after last reset), remaining = 100000
    remaining = sim.calculateRemainingMs(600000);
    ASSERT(remaining == 100000);

    // At time=700000 (300000ms after last reset), remaining = 0 (expired)
    remaining = sim.calculateRemainingMs(700000);
    ASSERT(remaining == 0);
}

// ============ Main ============

int main() {
    printf("=== test_inactivity_timeout ===\n");

    RUN_TEST(test_paired_timeout_half_elapsed);
    RUN_TEST(test_paired_timeout_zero_elapsed);
    RUN_TEST(test_paired_timeout_exact_expiry);
    RUN_TEST(test_unpaired_timeout_exceeded_by_one);
    RUN_TEST(test_unpaired_timeout_exact_expiry);
    RUN_TEST(test_unpaired_timeout_one_ms_before_expiry);
    RUN_TEST(test_unpaired_timeout_large_elapsed);
    RUN_TEST(test_activity_reset_recalculates_from_new_timer);
    RUN_TEST(test_activity_reset_with_paired_state_change);
    RUN_TEST(test_multiple_activity_resets_extend_deadline);

    TEST_SUMMARY();
}
