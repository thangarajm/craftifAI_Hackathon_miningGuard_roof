#include "roof_status_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

static portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
static roof_status_t s_status = ROOF_STATUS_SAFE;
static bool s_inited = false;

bool roof_status_service_init(void)
{
    if (s_inited) {
        return true;
    }
    s_status = ROOF_STATUS_SAFE;
    s_inited = true;
    return true;
}

void roof_status_service_set(roof_status_t status)
{
    portENTER_CRITICAL(&s_mux);
    s_status = status;
    portEXIT_CRITICAL(&s_mux);
}

roof_status_t roof_status_service_get(void)
{
    roof_status_t v;
    portENTER_CRITICAL(&s_mux);
    v = s_status;
    portEXIT_CRITICAL(&s_mux);
    return v;
}
