// ========== buzzer.h ==========
#ifndef BUZZER_H
#define BUZZER_H



#include "main.h"
#include "notes.h"



typedef struct
{
	TIM_HandleTypeDef *htim_buzzer;
	
	uint32_t channel;
	uint8_t state;
	uint32_t last_time_on;

	
	// melody fields
	const uint32_t    *melody_notes;
	const uint32_t    *melody_durations;
	uint8_t            melody_len;
	uint8_t            melody_index;
	uint8_t            is_playing_melody;
	uint32_t           note_start_time;
	uint32_t           note_duration;
	
}Buzzer;



void Buzzer_Init(Buzzer *buzzer, TIM_HandleTypeDef *buzzer_htim, uint32_t channel);

static void buzzer_set_state(Buzzer *buzzer, uint8_t state);

void Buzzer_Toggle(Buzzer *buzzer);
	
void Buzzer_BeepTick(Buzzer *buzzer, uint16_t period_ms);

void Buzzer_PlayNote(Buzzer *buzzer, uint32_t arr, uint32_t duration_ms);
	
void Buzzer_PlayMelody(Buzzer *buzzer, const uint32_t *notes,
                       const uint32_t *durations, uint8_t len);


void Buzzer_Update(Buzzer *buzzer);

void Buzzer_StartupSound(Buzzer *buzzer);

void Buzzer_Warning(Buzzer *buzzer);



#endif