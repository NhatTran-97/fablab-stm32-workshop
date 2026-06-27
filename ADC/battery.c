#include "battery.h"
#include "main.h"

#define BATTERY_Rtop 						20000.0f
#define BATTERY_Rbot 						10000.0f
#define BATTERY_Vref_VOLTS 			3.01f
#define BATTERY_ADC_MAX_COUNTS 			4095.0f

void battery_init(BATTERY *battery, ADC_HandleTypeDef *hadc)
{
    if (battery == NULL || hadc == NULL)
    {
        return;
    }

    battery->adc_bat = hadc;
		
		HAL_ADCEx_Calibration_Start(battery->adc_bat, ADC_SINGLE_ENDED); // Can chinh bo ADC
}

uint16_t get_adc_channel(BATTERY *battery, uint32_t channel)
{
    uint16_t adc = 0;

    if (battery == NULL || battery->adc_bat == NULL)
    {
        return 0;
    }

    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(battery->adc_bat, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_Start(battery->adc_bat) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_PollForConversion(battery->adc_bat, 1000) == HAL_OK)
    {
        adc = HAL_ADC_GetValue(battery->adc_bat);
    }

    HAL_ADC_Stop(battery->adc_bat);

    return adc;
}


float battery_get_adc_voltage(BATTERY *battery, uint32_t channel)
{
    uint16_t adc = get_adc_channel(battery, channel);

    return ((float)adc * BATTERY_Vref_VOLTS) / BATTERY_ADC_MAX_COUNTS;
}

float get_battery(BATTERY *battery, uint32_t channel)
{
	float adc_voltage = battery_get_adc_voltage(battery, channel);
	return adc_voltage * ((BATTERY_Rtop + BATTERY_Rbot) / BATTERY_Rbot);
}

