// ========== buzzer.c ==========
#include "buzzer.h"
#include "melody.h" 

#define BUZZER_OFF  0
#define BUZZER_ON   1



#define BUZZER_DUTY_PERCENT   40  



void Buzzer_Init(Buzzer *buzzer, TIM_HandleTypeDef *buzzer_htim, uint32_t channel)
{
		buzzer->state = BUZZER_OFF;

	  buzzer->last_time_on = HAL_GetTick();
	
		buzzer->htim_buzzer = buzzer_htim;
		buzzer->channel = channel;
	
	  buzzer->is_playing_melody = 0;
    buzzer->melody_index      = 0;
    buzzer->melody_notes      = NULL;
    buzzer->melody_durations  = NULL;
    buzzer->melody_len        = 0;
    buzzer->note_start_time   = 0;
    buzzer->note_duration     = 0;
	
    HAL_TIMEx_PWMN_Start(buzzer->htim_buzzer, buzzer->channel);  
}

// -------------------------------------------------------

static void Buzzer_SetState(Buzzer *buzzer, uint8_t state)
{
    buzzer->state = state;

    if (state == BUZZER_ON)
    {
        uint32_t arr = buzzer->htim_buzzer ->Instance->ARR;   //could be replace by __HAL_TIM_GET_AUTORELOAD(buzzer->htim_buzzer);
        uint32_t ccr = (arr + 1) * BUZZER_DUTY_PERCENT / 100;
        __HAL_TIM_SET_COMPARE(buzzer->htim_buzzer, buzzer->channel, ccr);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(buzzer->htim_buzzer, buzzer->channel, 0);
    }
}


void Buzzer_Toggle(Buzzer *buzzer)
{
    Buzzer_SetState(buzzer,(buzzer->state == BUZZER_ON) ? BUZZER_OFF : BUZZER_ON);
}


void Buzzer_BeepTick(Buzzer *buzzer, uint16_t period_ms)
{
	 if (buzzer->is_playing_melody) return;	
	
		uint32_t now = HAL_GetTick();

		if (now - buzzer->last_time_on >= period_ms)
		{
				Buzzer_Toggle(buzzer);  
				buzzer->last_time_on = now;
		}

}

void Buzzer_PlayNote(Buzzer *buzzer, uint32_t arr, uint32_t duration_ms)
{
    if (arr == NOTE_REST || arr == 0)
    {
        Buzzer_SetState(buzzer, BUZZER_OFF);
    }
    else
    {
        __HAL_TIM_SET_AUTORELOAD(buzzer->htim_buzzer, arr);
        Buzzer_SetState(buzzer, BUZZER_ON);
    }
    buzzer->note_start_time = HAL_GetTick();
    buzzer->note_duration   = duration_ms;
    buzzer->is_playing_melody = 1;   // lock BeepTick l?i khi dang phát nh?c
}

// ------------------------------------------------------------
void Buzzer_PlayMelody(Buzzer *buzzer, const uint32_t *notes,
                       const uint32_t *durations, uint8_t len)
{
    buzzer->melody_notes     = notes;
    buzzer->melody_durations = durations;
    buzzer->melody_len       = len;
    buzzer->melody_index     = 0;
	
    Buzzer_PlayNote(buzzer, notes[0], durations[0]);
}



void Buzzer_Update(Buzzer *buzzer)
{
    if (!buzzer->is_playing_melody) return;

    uint32_t now = HAL_GetTick();
    if (now - buzzer->note_start_time < buzzer->note_duration) return;

    buzzer->melody_index++;

    if (buzzer->melody_index < buzzer->melody_len)
    {

        Buzzer_PlayNote(buzzer,
            buzzer->melody_notes[buzzer->melody_index],
            buzzer->melody_durations[buzzer->melody_index]);
    }
    else
    {
        buzzer->is_playing_melody = 0;
        Buzzer_SetState(buzzer, BUZZER_OFF);
    }
}




void Buzzer_StartupSound(Buzzer *buzzer)
{
//    static const uint32_t notes[] = { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
//    static const uint32_t durs[] = {320, 320, 320, 300};
//    Buzzer_PlayMelody(buzzer, notes, durs, 4);
	
	    Buzzer_PlayMelody(buzzer,
															MELODY_STARTUP.notes,
															MELODY_STARTUP.durations,
															MELODY_STARTUP.len);
}

void Buzzer_Warning(Buzzer *buzzer)
{
    if (buzzer->is_playing_melody) return;

    Buzzer_PlayMelody(buzzer,
											MELODY_STAR_WARS_ALARM.notes,
											MELODY_STAR_WARS_ALARM.durations,
											MELODY_STAR_WARS_ALARM.len);
}


// MACRO Technique
//void Buzzer_Warning(Buzzer *buzzer)
//{
//    if (buzzer->is_playing_melody) return;
//    Buzzer_PlayMelody(buzzer, MELODY_ARGS(MELODY_STAR_WARS_ALARM));
//}




