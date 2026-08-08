#include "roof_monitor.h"

#include <string.h>

#include "app_config.h"
#include "logger.h"

#define TAG "roof_monitor"

typedef struct {
    roof_status_t status;
    float baseline_cm;
    float distance_cm;
    float delta_cm;

    // SAFE hysteresis
    int safe_consecutive;

    // delta history for ~1s lookback (APP_MEASUREMENT_PERIOD_MS * 5 = 1000ms)
    float delta_hist[5];
    uint8_t delta_hist_idx;
    bool delta_hist_full;

    int64_t last_reminder_ms;
    bool baseline_valid;
} roof_monitor_t;

static roof_monitor_t s_m;

static void log_state(const roof_monitor_snapshot_t *snap)
{
    switch (snap->status) {
    case ROOF_STATUS_SAFE:
        ESP_LOGI(TAG, "SAFE — distance: %.1fcm", (double)snap->distance_cm);
        break;
    case ROOF_STATUS_CAUTION:
        ESP_LOGW(TAG, "CAUTION — roof shifting, delta: %.1fcm", (double)snap->delta_cm);
        break;
    case ROOF_STATUS_DANGER:
        ESP_LOGE(TAG, "DANGER — imminent collapse, delta: %.1fcm", (double)snap->delta_cm);
        break;
    default:
        break;
    }
}

bool roof_monitor_init(void)
{
    memset(&s_m, 0, sizeof(s_m));
    s_m.status = ROOF_STATUS_SAFE;
    s_m.last_reminder_ms = 0;
    s_m.baseline_valid = false;
    return true;
}

static bool has_1s_history(void)
{
    return s_m.delta_hist_full;
}

static float delta_1s_ago(void)
{
    // Must only be called when has_1s_history() == true
    // 5 samples back is current index (idx) which points to next write.
    // The oldest sample is at idx.
    return s_m.delta_hist[s_m.delta_hist_idx];
}

static void push_delta(float d)
{
    s_m.delta_hist[s_m.delta_hist_idx] = d;
    s_m.delta_hist_idx = (uint8_t)((s_m.delta_hist_idx + 1) % 5);
    if (s_m.delta_hist_idx == 0) {
        s_m.delta_hist_full = true;
    }
}

void roof_monitor_update(float distance_cm, int64_t now_ms)
{
    if (!s_m.baseline_valid) {
        // Should be set by baseline procedure; keep SAFE until then.
        s_m.distance_cm = distance_cm;
        s_m.baseline_cm = distance_cm;
        s_m.delta_cm = 0.0f;
        return;
    }

    s_m.distance_cm = distance_cm;
    s_m.delta_cm = s_m.baseline_cm - s_m.distance_cm;
    // Only decreases (delta>0) are meaningful for CAUTION/DANGER. Store negative too for history,
    // but never use negative values to trigger alerts.
    push_delta(s_m.delta_cm);

    const float abs_diff = (s_m.distance_cm > s_m.baseline_cm)
                               ? (s_m.distance_cm - s_m.baseline_cm)
                               : (s_m.baseline_cm - s_m.distance_cm);

    roof_status_t prev = s_m.status;

    // SAFE band with hysteresis
    if (abs_diff <= APP_SAFE_BAND_CM) {
        s_m.safe_consecutive++;
    } else {
        s_m.safe_consecutive = 0;
    }

    if (s_m.safe_consecutive >= APP_SAFE_CONSECUTIVE_SAMPLES) {
        s_m.status = ROOF_STATUS_SAFE;
    } else {
        // consider CAUTION / DANGER only for decreases (positive delta)
        if (s_m.delta_cm > APP_CAUTION_DELTA_CM) {
            // Only evaluate DANGER "rate" once we have a full 1s history buffer
            if (has_1s_history()) {
                float d_ago = delta_1s_ago();
                float jump = s_m.delta_cm - d_ago;
                if (s_m.delta_cm > APP_DANGER_DELTA_CM && jump > APP_DANGER_DELTA_CM) {
                    s_m.status = ROOF_STATUS_DANGER;
                } else {
                    s_m.status = ROOF_STATUS_CAUTION;
                }
            } else {
                s_m.status = ROOF_STATUS_CAUTION;
            }
        } else {
            // below caution threshold; allow to settle to SAFE via hysteresis
            if (prev == ROOF_STATUS_DANGER || prev == ROOF_STATUS_CAUTION) {
                s_m.status = prev;
            }
        }
    }

    roof_monitor_snapshot_t snap = {
        .status = s_m.status,
        .baseline_cm = s_m.baseline_cm,
        .distance_cm = s_m.distance_cm,
        .delta_cm = s_m.delta_cm,
    };

    const bool state_changed = (s_m.status != prev);
    const bool needs_reminder = (s_m.status == ROOF_STATUS_CAUTION || s_m.status == ROOF_STATUS_DANGER);

    if (state_changed) {
        log_state(&snap);
        s_m.last_reminder_ms = now_ms;
    } else if (needs_reminder && (now_ms - s_m.last_reminder_ms) >= APP_REMINDER_PERIOD_MS) {
        log_state(&snap);
        s_m.last_reminder_ms = now_ms;
    }
}

roof_monitor_snapshot_t roof_monitor_get_snapshot(void)
{
    roof_monitor_snapshot_t snap = {
        .status = s_m.status,
        .baseline_cm = s_m.baseline_cm,
        .distance_cm = s_m.distance_cm,
        .delta_cm = s_m.delta_cm,
    };
    return snap;
}

// Baseline setter used by app logic
void roof_monitor_set_baseline(float baseline_cm)
{
    s_m.baseline_cm = baseline_cm;
    s_m.baseline_valid = true;
    s_m.safe_consecutive = APP_SAFE_CONSECUTIVE_SAMPLES;
    s_m.status = ROOF_STATUS_SAFE;
    s_m.delta_cm = 0.0f;
    memset(s_m.delta_hist, 0, sizeof(s_m.delta_hist));
    s_m.delta_hist_idx = 0;
    s_m.delta_hist_full = false;
}
