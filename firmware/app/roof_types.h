#ifndef ROOF_TYPES_H
#define ROOF_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ROOF_STATUS_SAFE = 0,
    ROOF_STATUS_CAUTION,
    ROOF_STATUS_DANGER,
} roof_status_t;

#ifdef __cplusplus
}
#endif

#endif /* ROOF_TYPES_H */
