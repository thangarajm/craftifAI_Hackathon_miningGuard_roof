#ifndef ROOF_STATUS_SERVICE_H
#define ROOF_STATUS_SERVICE_H

#include <stdbool.h>

#include "roof_types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool roof_status_service_init(void);

void roof_status_service_set(roof_status_t status);
roof_status_t roof_status_service_get(void);

#ifdef __cplusplus
}
#endif

#endif /* ROOF_STATUS_SERVICE_H */
