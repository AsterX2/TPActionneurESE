/*
 * shell.c
 *
 *  Created on: Oct 1, 2023
 *      Author: nicolas
 */
#include "usart.h"
#include "mylibs/shell.h"
#include "tim.h"
#include <stdlib.h>
#include "mylibs/cmd_speed.h"
#include "adc.h"

//#include "../tim.c"

#include <stdio.h>
#include <string.h>

uint8_t prompt[]="user@Nucleo-STM32G474RET6>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
uint8_t brian[]="Brian is in the kitchen\r\n";
uint8_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

//extern TIM_HandleTypeDef htim1;
uint16_t adcBuffer[BUFFER_SIZE]; // Buffer pour stocker les données ADC
char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;

void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

/*void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: // Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){
		if(strcmp(argv[0],"WhereisBrian?")==0){
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0],"help")==0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Print all available functions here\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0], "speed") == 0) {
			if(argc > 1)
			{  // Assurez-vous qu'il y a un argument après "speed"
				int speedValue = atoi(argv[1]);  // Convertir l'argument en entier
				// Faites quelque chose avec speed_value, comme l'afficher ou le traiter : on change le duty cycle
				//if (speedValue < 0) speedValue = 0;
				if ((speedValue) > 200  & (speedValue) < 2000)
				{
					//MaJ du rapport cylique
					//uint32_t dutyCycle = 614;  // Exemple de valeur de rapport cyclique (60% si la période est 1023)

					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speedValue);//alpha PWM channel 1 changé (U_High).Le complémentaire se change de lui-même par définition
					//Il faudra changer le rapport cyclique de l'autre channel (channel 2)


					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, TIM1->ARR-speedValue); //V_High
				}



				else{


					HAL_UART_Transmit(&huart2, (uint8_t *)"Invalid speed value\r\n", 30, HAL_MAX_DELAY);
				}

			}

		}
		else {
			HAL_UART_Transmit(&huart2, (uint8_t *)"Please provide a speed value\r\n", 30, HAL_MAX_DELAY);
		}
	}

	else{
		HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
	newCmdReady = 0;
}*/
void Shell_Loop(void){


	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token != NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: // Suppression du dernier caractère
			if (idx_cmd > 0) { // Eviter les index négatifs
				cmdBuffer[--idx_cmd] = '\0';
				HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			}
			break;
		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){
		int commandRecognized = 0; // Indicateur pour vérifier si une commande est reconnue

		if(strcmp(argv[0], "WhereisBrian?") == 0){
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
			commandRecognized = 1;
		}
		else if(strcmp(argv[0], "help") == 0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Print all available functions here\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			commandRecognized = 1;
		}
		else if( strcmp(argv[0], "start") == 0){
			start_PWM (htim1,TIM_CHANNEL_1);
			commandRecognized = 1;


		}
		else if(strcmp(argv[0], "stop") == 0 ){
			stop_PWM (htim1,TIM_CHANNEL_1);
			stop_PWM (htim1,TIM_CHANNEL_2);
			commandRecognized = 1;

		}
		else if(strcmp(argv[0], "current") == 0) {

			//uint8_t BUFFER_SIZE=5;
			//uint16_t adcBuff[BUFFER_SIZE];
			//HAL_ADC_Start_DMA(&hadc1,adcBuff,BUFFER_SIZE);

			//HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
			//uint16_t adcBuff;
			//adcBuff=HAL_ADC_GetValue(&hadc1);

			float sentivity=0.05;
			float adc_val_max=4096.0;
			float  adc_vcc =3.3;
			int offset =0;

			float u_adc= adc_vcc * ( (int) (adcBuffer[0]) - offset )/ adc_val_max;
			float Imes=(u_adc-1.65)/sentivity;


			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Valeur : %d \r\n", adcBuffer[0]);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);

			uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "ValeurImes : %.2f \r\n", Imes);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);

		}

		else if(strcmp(argv[0], "speed") == 0) {
			if(argc > 1) {
				int speedValue = atoi(argv[1]);
				if(speedValue >= 200 && speedValue <= 2000){
					//On met le rapport cyclique à la valeur du rapport cyclique renseigné
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speedValue);
					__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, TIM1->ARR - speedValue);
					commandRecognized = 1;
				} else {
					HAL_UART_Transmit(&huart2, (uint8_t *)"Invalid speed value\r\n", strlen("Invalid speed value\r\n"), HAL_MAX_DELAY);
				}
			}
			else {
				HAL_UART_Transmit(&huart2, (uint8_t *)"Please provide a speed value\r\n", strlen("Please provide a speed value\r\n"), HAL_MAX_DELAY);
			}

			/*HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
			int value = HAL_ADC_GetValue(&hadc1);
			float Imes = (value-((1,65))/(0,05));
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Valeur : %4d \r\n", value);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "ValeurImes : %4d \r\n", Imes);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);*/
		}

		// Si aucune commande n'est reconnue, envoyer "Command not found"
		if (!commandRecognized) {
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}

		newCmdReady = 0; // Réinitialiser seulement après exécution ou rejet de la commande
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
	}

}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
