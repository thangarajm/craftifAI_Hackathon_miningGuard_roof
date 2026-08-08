#include "app.h"

#include "app_config.h"
#include "hcsr04.h"
#include "logger.h"
#include "rgb_led.h"
#include "roof_monitor.h"
#include "roof_monitor_baseline.h"
#include "roof_status_service.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app";

typedef struct {
    roof_status_t status;
} shared_state_t;

static shared_state_t s_shared = {
    .status = ROOF_STATUS_SAFE,
};

static void sensor_task(void *arg)
{
    (void)arg;

    hcsr04_config_t cfg = {
        .trig_gpio = APP_HCSR04_TRIG_GPIO,
        .echo_gpio = APP_HCSR04_ECHO_GPIO,
    };
    if (!hcsr04_init(&cfg)) {
        ESP_LOGE(TAG, "HC-SR04 init failed");
        vTaskSuspend(NULL);
    }

    float baseline_cm = 0.0f;
    if (!roof_monitor_acquire_baseline(&baseline_cm)) {
        ESP_LOGE(TAG, "Unable to acquire baseline. Holding SAFE but measurements may be invalid.");
        // fallback: keep trying forever
        while (!roof_monitor_acquire_baseline(&baseline_cm)) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    roof_monitor_set_baseline(baseline_cm);

    float last_valid[3] = {0};
    int last_valid_count = 0;
    float prev_filtered = 0.0f;
    bool prev_filtered_valid = false;

    while (1) {
        float cm = 0.0f;
        bool ok = hcsr04_read_cm(&cm);

        if (ok) {
            // Median-of-3 filter (use what we have)
            last_valid[last_valid_count % 3] = cm;
            last_valid_count++;

            float filtered = cm;
            const int n = (last_valid_count < 3) ? last_valid_count : 3;
            if (n == 3) {
                float a = last_valid[0], b = last_valid[1], c = last_valid[2];
                if (a > b) { float t = a; a = b; b = t; }
                if (b > c) { float t = b; b = c; c = t; }
                if (a > b) { float t = a; a = b; b = t; }
                filtered = b;
            }

            // Reject unrealistic jumps (typically garbage pulse widths)
            if (prev_filtered_valid) {
                float diff = filtered - prev_filtered;
                if (diff < 0) diff = -diff;
                if (diff > APP_MAX_JUMP_CM_PER_SAMPLE) {
                    ok = false;
                }
            }

            if (ok) {
                prev_filtered = filtered;
                prev_filtered_valid = true;

                int64_t now_ms = esp_timer_get_time() / 1000;
                roof_monitor_update(filtered, now_ms);
            }

            roof_monitor_snapshot_t snap = roof_monitor_get_snapshot();
            s_shared.status = snap.status;
            roof_status_service_set(snap.status);
        }

        vTaskDelay(pdMS_TO_TICKS(APP_MEASUREMENT_PERIOD_MS));
    }
}

static void led_task(void *arg)
{
    (void)arg;

    if (!rgb_led_init()) {
        ESP_LOGE(TAG, "RGB LED init failed");
        vTaskSuspend(NULL);
    }

    // Simple blink generator
    bool on = false;
    int64_t last_toggle_ms = 0;

    while (1) {
        roof_status_t st = roof_status_service_get();
        int64_t now_ms = esp_timer_get_time() / 1000;

        if (st == ROOF_STATUS_SAFE) {
            rgb_led_set((rgb_led_color_t){.r = 0, .g = 255, .b = 0});
            on = true;
            last_toggle_ms = now_ms;
        } else if (st == ROOF_STATUS_CAUTION) {
            const int period_ms = 1000 / APP_CAUTION_BLINK_HZ;
            if ((now_ms - last_toggle_ms) >= (period_ms / 2)) {
                on = !on;
                last_toggle_ms = now_ms;
            }
            if (on) {
                rgb_led_set((rgb_led_color_t){.r = 255, .g = 120, .b = 0});
            } else {
                rgb_led_off();
            }
        } else { // DANGER
            const int period_ms = 1000 / APP_DANGER_BLINK_HZ;
            if ((now_ms - last_toggle_ms) >= (period_ms / 2)) {
                on = !on;
                last_toggle_ms = now_ms;
            }
            if (on) {
                rgb_led_set((rgb_led_color_t){.r = 255, .g = 0, .b = 0});
            } else {
                rgb_led_off();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(APP_LED_TASK_PERIOD_MS));
    }
}

void app_start(void)
{
    ESP_LOGI(TAG, "Mine roof monitor starting");

    (void)roof_status_service_init();
    (void)roof_monitor_init();

    xTaskCreate(sensor_task, "sensor", 4096, NULL, 5, NULL);
    xTaskCreate(led_task, "led", 3072, NULL, 4, NULL);
}
