/**
 * test_motor_mapping.cpp
 *
 * Tests the motor direction-to-speed mapping logic extracted from
 * MotorDriveController::write() in Perse_Rover-Firmware.
 *
 * Validates Requirements: 15.1, 15.2, 15.3
 *
 * Compile:
 *   g++ -std=c++17 -I../../Perse-Common/test -o test_motor_mapping test_motor_mapping.cpp && ./test_motor_mapping
 */

#include "freertos_mock.h"
#include <algorithm> // std::clamp
#include <cmath>     // fabs

// ============================================================================
// Extracted pure mapping logic from MotorDriveController::write()
// ============================================================================

struct MotorSpeeds {
    float left;
    float right;
};

/**
 * Maps a direction (0-7) and speed (0.0-1.0) to left/right motor values
 * in the range [-100, 100].
 *
 * Extracted from MotorDriveController::write() for testability.
 */
MotorSpeeds mapMotorSpeeds(uint8_t dir, float speed) {
    float leftSpeed = 0.0f;
    float rightSpeed = 0.0f;

    if (dir == 0) {
        leftSpeed = rightSpeed = 100;
    } else if (dir == 1) {
        leftSpeed = 100;
        rightSpeed = 30;
    } else if (dir == 2) {
        leftSpeed = 100;
        rightSpeed = -100;
    } else if (dir == 3) {
        leftSpeed = -100;
        rightSpeed = -30;
    } else if (dir == 4) {
        leftSpeed = rightSpeed = -100;
    } else if (dir == 5) {
        leftSpeed = -30;
        rightSpeed = -100;
    } else if (dir == 6) {
        leftSpeed = -100;
        rightSpeed = 100;
    } else if (dir == 7) {
        leftSpeed = 30;
        rightSpeed = 100;
    }

    leftSpeed = std::clamp(leftSpeed * speed, -100.0f, 100.0f);
    rightSpeed = std::clamp(rightSpeed * speed, -100.0f, 100.0f);

    return {leftSpeed, rightSpeed};
}

// ============================================================================
// Tests
// ============================================================================

static bool floatEq(float a, float b, float eps = 0.001f) {
    return fabs(a - b) < eps;
}

/**
 * Requirement 15.1: FOR ALL directions 0-7 with speed=1.0,
 * the Motor_Mapper SHALL produce the expected left/right pairs.
 */
void test_all_directions_speed_1() {
    struct TestCase {
        uint8_t dir;
        float expectedLeft;
        float expectedRight;
    };

    TestCase cases[] = {
        {0, 100.0f, 100.0f},
        {1, 100.0f, 30.0f},
        {2, 100.0f, -100.0f},
        {3, -100.0f, -30.0f},
        {4, -100.0f, -100.0f},
        {5, -30.0f, -100.0f},
        {6, -100.0f, 100.0f},
        {7, 30.0f, 100.0f},
    };

    for (const auto& tc : cases) {
        MotorSpeeds result = mapMotorSpeeds(tc.dir, 1.0f);
        ASSERT(floatEq(result.left, tc.expectedLeft));
        ASSERT(floatEq(result.right, tc.expectedRight));
    }
}

/**
 * Requirement 15.2: WHEN speed is 0.0 for any direction,
 * the Motor_Mapper SHALL produce left=0 and right=0.
 */
void test_all_directions_speed_0() {
    for (uint8_t dir = 0; dir < 8; dir++) {
        MotorSpeeds result = mapMotorSpeeds(dir, 0.0f);
        ASSERT(floatEq(result.left, 0.0f));
        ASSERT(floatEq(result.right, 0.0f));
    }
}

/**
 * Requirement 15.3: FOR ALL directions and speeds,
 * the Motor_Mapper SHALL produce values clamped to [-100, 100].
 */
void test_output_clamped() {
    // Test with various speeds including >1.0 to verify clamping
    float speeds[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 10.0f};

    for (uint8_t dir = 0; dir < 8; dir++) {
        for (float speed : speeds) {
            MotorSpeeds result = mapMotorSpeeds(dir, speed);
            ASSERT(result.left >= -100.0f);
            ASSERT(result.left <= 100.0f);
            ASSERT(result.right >= -100.0f);
            ASSERT(result.right <= 100.0f);
        }
    }
}

/**
 * Additional: Verify half-speed produces proportional output.
 */
void test_half_speed_proportional() {
    // At speed=0.5, outputs should be half the full-speed values
    struct TestCase {
        uint8_t dir;
        float expectedLeft;
        float expectedRight;
    };

    TestCase cases[] = {
        {0, 50.0f, 50.0f},
        {1, 50.0f, 15.0f},
        {2, 50.0f, -50.0f},
        {3, -50.0f, -15.0f},
        {4, -50.0f, -50.0f},
        {5, -15.0f, -50.0f},
        {6, -50.0f, 50.0f},
        {7, 15.0f, 50.0f},
    };

    for (const auto& tc : cases) {
        MotorSpeeds result = mapMotorSpeeds(tc.dir, 0.5f);
        ASSERT(floatEq(result.left, tc.expectedLeft));
        ASSERT(floatEq(result.right, tc.expectedRight));
    }
}

/**
 * Additional: Verify invalid direction (>7) produces zero output.
 */
void test_invalid_direction() {
    // Directions > 7 don't match any if branch, so leftSpeed/rightSpeed stay 0
    for (uint8_t dir = 8; dir < 16; dir++) {
        MotorSpeeds result = mapMotorSpeeds(dir, 1.0f);
        ASSERT(floatEq(result.left, 0.0f));
        ASSERT(floatEq(result.right, 0.0f));
    }
}

int main() {
    printf("=== Motor Mapping Tests ===\n");

    RUN_TEST(test_all_directions_speed_1);
    RUN_TEST(test_all_directions_speed_0);
    RUN_TEST(test_output_clamped);
    RUN_TEST(test_half_speed_proportional);
    RUN_TEST(test_invalid_direction);

    TEST_SUMMARY();
}
