#ifndef ROOF_MONITOR_H
#define ROOF_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

#include "roof_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "roof_types.h"

typedef struct {
    roof_status_t status;
    float baseline_cm;
    float distance_cm;
    float delta_cm; // baseline - distance (positive means decreased distance)
} roof_monitor_snapshot_t;

bool roof_monitor_init(void);

/**
 * @brief Update monitor with a new distance sample (cm)
 */
void roof_monitor_update(float distance_cm, int64_t now_ms);

roof_monitor_snapshot_t roof_monitor_get_snapshot(void);

/**
 * @brief Set baseline distance in centimeters.
 *
 * Called once after startup baseline acquisition.
 */
void roof_monitor_set_baseline(float baseline_cm);

#ifdef __cplusplus
}
#endif

#endif /* ROOF_MONITOR_H */
