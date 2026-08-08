#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_led_color_t;

bool rgb_led_init(void);
void rgb_led_set(rgb_led_color_t c);
void rgb_led_off(void);

#ifdef __cplusplus
}
#endif

#endif /* RGB_LED_H */
