#ifndef HCSR04_H
#define HCSR04_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int trig_gpio;
    int echo_gpio;
} hcsr04_config_t;

/**
 * @brief Initialize HC-SR04 GPIOs
 */
bool hcsr04_init(const hcsr04_config_t *cfg);

/**
 * @brief Read distance in centimeters
 *
 * @param out_cm Output distance in centimeters
 * @return true if a valid reading was captured, false on timeout/invalid/out-of-range
 */
bool hcsr04_read_cm(float *out_cm);

#ifdef __cplusplus
}
#endif

#endif /* HCSR04_H */
