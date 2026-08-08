#include "roof_monitor_baseline.h"

#include <string.h>

#include "app_config.h"
#include "hcsr04.h"
#include "logger.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdlib.h>

#define TAG "baseline"

static int cmp_float(const void *a, const void *b)
{
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    if (fa < fb) return -1;
    if (fa > fb) return 1;
    return 0;
}

bool roof_monitor_acquire_baseline(float *out_baseline_cm)
{
    if (out_baseline_cm == NULL) {
        return false;
    }

    float samples[APP_BASELINE_SAMPLE_COUNT];
    int valid = 0;
    const int64_t start_ms = esp_timer_get_time() / 1000;

    while (1) {
        float cm = 0.0f;
        if (hcsr04_read_cm(&cm)) {
            if (valid < APP_BASELINE_SAMPLE_COUNT) {
                samples[valid++] = cm;
            }
        }

        const int64_t now_ms = esp_timer_get_time() / 1000;
        const int64_t elapsed_ms = now_ms - start_ms;

        if (valid >= APP_BASELINE_SAMPLE_COUNT) {
            break;
        }

        if (elapsed_ms >= APP_BASELINE_MAX_TIME_MS && valid >= APP_BASELINE_MIN_VALID_SAMPLES) {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(APP_MEASUREMENT_PERIOD_MS));
    }

    if (valid < APP_BASELINE_MIN_VALID_SAMPLES) {
        ESP_LOGE(TAG, "Baseline failed: only %d valid samples", valid);
        return false;
    }

    // compute median of collected valid samples
    qsort(samples, valid, sizeof(samples[0]), cmp_float);
    float median = samples[valid / 2];

    *out_baseline_cm = median;
    ESP_LOGI(TAG, "Baseline established: %.1fcm (%d samples)", (double)median, valid);
    return true;
}
