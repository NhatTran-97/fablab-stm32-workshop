#include "melody.h"


// =====================================================
//  STARTUP
// =====================================================
static const uint32_t _startup_notes[] = {
    NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6
};
static const uint32_t _startup_dur[] = {
    120, 120, 120, 200
};
const Melody MELODY_STARTUP = {
    .notes     = _startup_notes,
    .durations = _startup_dur,
    .len       = sizeof(_startup_notes) / sizeof(_startup_notes[0]),
    .name      = "Startup"
};


// =====================================================
//  STAR WARS ALARM ??
// =====================================================


static const uint32_t _sw_notes[] = {
																		NOTE_A4,  NOTE_A4,  NOTE_A4,  NOTE_F4,  NOTE_C5,
																		NOTE_A4,  NOTE_A4,  NOTE_A4,  NOTE_F4,  NOTE_C5,
																		NOTE_A5,  NOTE_A4,  NOTE_A4,  NOTE_A5,  NOTE_Gs5,
																		NOTE_G5,  NOTE_Fs5, NOTE_E5,
																		NOTE_F5,  NOTE_REST,
																		NOTE_Bb4, NOTE_Eb5, NOTE_REST,
																		NOTE_D5,  NOTE_Db5,
																		NOTE_C5,  NOTE_REST,
};
static const uint32_t _sw_dur[] = {
																		150, 150, 150, 100, 400,
																		150, 150, 150, 100, 400,
																		150, 100, 150, 400, 400,
																		150, 150, 150,
																		300, 80,
																		150, 400, 80,
																		300, 300,
																		500, 200,
};

const Melody MELODY_STAR_WARS_ALARM = 
{
																		.notes     = _sw_notes,
																		.durations = _sw_dur,
																		.len       = sizeof(_sw_notes) / sizeof(_sw_notes[0]),
																		.name      = "Star Wars Alarm"
};

