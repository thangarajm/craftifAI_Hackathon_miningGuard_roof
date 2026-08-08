#ifndef ROOF_MONITOR_BASELINE_H
#define ROOF_MONITOR_BASELINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Acquire a stable baseline using median of APP_BASELINE_SAMPLE_COUNT valid samples.
 *
 * Blocks (delays) until enough samples are collected or a max time elapses.
 */
bool roof_monitor_acquire_baseline(float *out_baseline_cm);

#ifdef __cplusplus
}
#endif

#endif /* ROOF_MONITOR_BASELINE_H */
