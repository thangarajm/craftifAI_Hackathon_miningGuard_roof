#include "rgb_led.h"

#include "app_config.h"
#include "logger.h"

#include "led_strip.h"
#include "led_strip_rmt.h"

#define TAG "rgb_led"

static led_strip_handle_t s_strip = NULL;
static bool s_inited = false;

bool rgb_led_init(void)
{
    if (s_inited) {
        return true;
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = APP_WS2812_GPIO,
        .max_leds = APP_WS2812_LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = 0,
        },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 0,
        .flags = {
            .with_dma = 0,
        },
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "led_strip_new_rmt_device failed: %s", esp_err_to_name(err));
        return false;
    }

    (void)led_strip_clear(s_strip);
    (void)led_strip_refresh(s_strip);

    s_inited = true;
    ESP_LOGI(TAG, "WS2812 initialized on GPIO %d", APP_WS2812_GPIO);
    return true;
}

void rgb_led_set(rgb_led_color_t c)
{
    if (!s_inited || s_strip == NULL) {
        return;
    }

    // Note: API uses RGB, but common WS2812 expects GRB order; we configured GRB format.
    (void)led_strip_set_pixel(s_strip, 0, c.r, c.g, c.b);
    (void)led_strip_refresh(s_strip);
}

void rgb_led_off(void)
{
    if (!s_inited || s_strip == NULL) {
        return;
    }
    (void)led_strip_clear(s_strip);
    (void)led_strip_refresh(s_strip);
}
