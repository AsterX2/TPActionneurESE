/*
 * cmd_speed.c
 *
 *  Created on: Nov 13, 2024
 *      Author: MundusEst
 */
#include <stdint.h>
#include "tim.h"

#define alpha_MAX 1913   // Définir la valeur maximale du duty cycle à 90% (par exemple 10 bits -> 1023)
#define alpha_MID 1063	//Définir la valeur du rapport cyclique à 50%
#define RAMP_DELAY 10   // Temps en millisecondes entre chaque incrément
#define RAMP_STEP 5     // Valeur d'incrément du rapport cyclique à chaque étape

extern ADC_HandleTypeDef hadc1;
//extern adcBuffer;
void start_PWM (TIM_HandleTypeDef htim,uint32_t channel)
{

	HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim,TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim, TIM_CHANNEL_2);


	//On met le rapport cyclique à la valeur du rapport cyclique à 50%

	int alpha = alpha_MID;
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, alpha);// duty cycle channel 1 égal à 50%
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, TIM1->ARR - alpha); // duty cycle channel 2 égal à 50%

}

void stop_PWM(TIM_HandleTypeDef htim,uint32_t channel)

{
	HAL_TIM_PWM_Stop(&htim, channel);

	HAL_TIMEx_PWMN_Stop(&htim, channel);



}


/*float calc_I(uint32_t adc_buf,int adc_res)//calculer la valeur du courant à partir de la mesure de l'adc
{
	(int) adc_buf=(adc_buf)/(adc_res);


	 * u=3,3*mes_adc/4096
	 *
	 * u
	 *
	int value_u = (3.3*u)/4096;
	float Imes = (value_u -((1,65))/(0,05));
	return Imes;




	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	int value = HAL_ADC_GetValue(&hadc1);
	float Imes = (value-((1,65))/(0,05));
	int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Valeur adc lue: %4d \r\n", value);
	HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
	uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "ValeurImes post calc: %4d \r\n", Imes);
	HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);



}*/





/*

void speed_PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t target_speed) {
    // Limiter la valeur de vitesse cible à PWM_MAX
    if (target_speed > _MAX) {
        target_speed = PWM_MAX;
    }

    // Obtenir la valeur actuelle du rapport cyclique
    uint32_t current_speed = __HAL_TIM_GET_COMPARE(htim, channel);

    // Déterminer la direction de la rampe (montée ou descente)
    if (current_speed < target_speed) {
        // Rampe montante
        while (current_speed < target_speed) {
            current_speed += RAMP_STEP;
            if (current_speed > target_speed) {
                current_speed = target_speed; // S'assurer de ne pas dépasser la cible
            }
            __HAL_TIM_SET_COMPARE(htim, channel, current_speed);
            HAL_Delay(RAMP_DELAY); // Délai entre chaque étape de la rampe
        }
    } else {
        // Rampe descendante
        while (current_speed > target_speed) {
            current_speed -= RAMP_STEP;
            if (current_speed < target_speed) {
                current_speed = target_speed; // S'assurer de ne pas passer en dessous de la cible
            }
            __HAL_TIM_SET_COMPARE(htim, channel, current_speed);
            HAL_Delay(RAMP_DELAY); // Délai entre chaque étape de la rampe
        }
    }
}
 */

