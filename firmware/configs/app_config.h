#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Application-level compile-time constants: GPIO pins, buffer sizes, periods, etc.
   Add #define macros here; every source file that uses them includes "app_config.h". */

// HC-SR04 ultrasonic sensor
#define APP_HCSR04_TRIG_GPIO            4
#define APP_HCSR04_ECHO_GPIO            5

// On-board WS2812 (RGB) LED data pin (ESP32-C3-DevKitM-1)
#define APP_WS2812_GPIO                 8
#define APP_WS2812_LED_COUNT            1

// Sampling / timing
#define APP_MEASUREMENT_PERIOD_MS       200
#define APP_LED_TASK_PERIOD_MS          50

// HC-SR04 measurement behaviour
#define APP_HCSR04_TRIGGER_PULSE_US     10
#define APP_HCSR04_ECHO_RISE_TIMEOUT_US 30000   // wait for rising edge
#define APP_HCSR04_ECHO_HIGH_TIMEOUT_US 30000   // measure high pulse width
#define APP_HCSR04_MIN_CM               2.0f
#define APP_HCSR04_MAX_CM               400.0f

// Baseline capture
#define APP_BASELINE_SAMPLE_COUNT       25      // ~5s at 200ms
#define APP_BASELINE_MIN_VALID_SAMPLES  10
#define APP_BASELINE_MAX_TIME_MS        20000

// Thresholds
#define APP_CAUTION_DELTA_CM            2.0f
#define APP_DANGER_DELTA_CM             2.0f
#define APP_DANGER_RATE_WINDOW_MS       1000
#define APP_SAFE_BAND_CM                1.0f
#define APP_SAFE_CONSECUTIVE_SAMPLES    3

// Sample sanity gate (reject sudden jumps likely from bad echo captures)
#define APP_MAX_JUMP_CM_PER_SAMPLE      30.0f

// Logging
#define APP_REMINDER_PERIOD_MS          5000

// LED blink rates
#define APP_CAUTION_BLINK_HZ            1
#define APP_DANGER_BLINK_HZ             4

#endif /* APP_CONFIG_H */
