#ifndef MELODY_H
#define MELODY_H

#include "notes.h"
#include <stdint.h>



typedef struct {
    const uint32_t *notes;
    const uint32_t *durations;
    uint8_t         len;
    const char     *name;
} Melody;

// MACRO technique
// #define MELODY_ARGS(m)  (m).notes, (m).durations, (m).len

extern const Melody MELODY_STARTUP;
extern const Melody MELODY_STAR_WARS_ALARM;


#endif