#include "hcsr04.h"

#include <stddef.h>

#include "app_config.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "logger.h"

#define TAG "hcsr04"

static int s_trig_gpio = -1;
static int s_echo_gpio = -1;
static bool s_inited = false;

static inline void delay_us(uint32_t us)
{
    // vTaskDelay is too coarse; use esp_rom for short waits
    esp_rom_delay_us(us);
}

bool hcsr04_init(const hcsr04_config_t *cfg)
{
    if (s_inited) {
        return true;
    }
    if (cfg == NULL) {
        return false;
    }

    s_trig_gpio = cfg->trig_gpio;
    s_echo_gpio = cfg->echo_gpio;

    gpio_config_t trig = {
        .pin_bit_mask = (1ULL << s_trig_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config_t echo = {
        .pin_bit_mask = (1ULL << s_echo_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    if (gpio_config(&trig) != ESP_OK || gpio_config(&echo) != ESP_OK) {
        return false;
    }

    gpio_set_level(s_trig_gpio, 0);

    s_inited = true;
    ESP_LOGI(TAG, "Initialized (TRIG=%d, ECHO=%d)", s_trig_gpio, s_echo_gpio);
    return true;
}

static bool wait_for_level(int gpio_num, int level, int64_t timeout_us)
{
    const int64_t start = esp_timer_get_time();
    while ((esp_timer_get_time() - start) < timeout_us) {
        if (gpio_get_level(gpio_num) == level) {
            return true;
        }
        // tiny delay to reduce CPU burn while still keeping resolution
        delay_us(2);
    }
    return false;
}

bool hcsr04_read_cm(float *out_cm)
{
    if (!s_inited || out_cm == NULL) {
        return false;
    }

    // Ensure a clean low period before triggering
    gpio_set_level(s_trig_gpio, 0);
    delay_us(2);

    // Trigger pulse
    gpio_set_level(s_trig_gpio, 1);
    delay_us(APP_HCSR04_TRIGGER_PULSE_US);
    gpio_set_level(s_trig_gpio, 0);

    // Wait for rising edge
    if (!wait_for_level(s_echo_gpio, 1, APP_HCSR04_ECHO_RISE_TIMEOUT_US)) {
        return false;
    }
    const int64_t t_rise = esp_timer_get_time();

    // Wait for falling edge
    if (!wait_for_level(s_echo_gpio, 0, APP_HCSR04_ECHO_HIGH_TIMEOUT_US)) {
        return false;
    }
    const int64_t t_fall = esp_timer_get_time();

    const int64_t pulse_us = t_fall - t_rise;
    if (pulse_us <= 0) {
        return false;
    }

    // Convert pulse width to distance.
    // Distance (cm) = (pulse_us * speed_of_sound_cm_per_us) / 2
    // speed_of_sound = 343 m/s = 34300 cm/s = 0.0343 cm/us
    const float cm = (float)pulse_us * 0.0343f * 0.5f;

    if (cm < APP_HCSR04_MIN_CM || cm > APP_HCSR04_MAX_CM) {
        return false;
    }

    *out_cm = cm;
    return true;
}
