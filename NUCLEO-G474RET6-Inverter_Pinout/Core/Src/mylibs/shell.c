/**
 * @file shell.c
 * @brief Implémentation d'un shell de commandes pour le microcontrôleur STM32G474RET6
 * @author nicolas
 * @date 1 Octobre 2023
 * 
 * Ce fichier implémente un shell interactif permettant à l'utilisateur d'interagir
 * avec le microcontrôleur via une interface UART. Il gère différentes commandes
 * comme le contrôle de la vitesse du moteur et la lecture du courant.
 */

#include "usart.h"
#include "mylibs/shell.h"
#include "tim.h"
#include <stdlib.h>
#include "mylibs/cmd_speed.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>

/** @brief Prompt affiché avant chaque commande */
uint8_t prompt[] = "user@Nucleo-STM32G474RET6>>";

/** @brief Message de bienvenue affiché au démarrage */
uint8_t started[] = 
    "\r\n*-----------------------------*"
    "\r\n| Welcome on Nucleo-STM32G474 |"
    "\r\n*-----------------------------*"
    "\r\n";

/** @brief Caractères de nouvelle ligne */
uint8_t newline[] = "\r\n";

/** @brief Caractères pour effacer le dernier caractère */
uint8_t backspace[] = "\b \b";

/** @brief Message d'erreur pour une commande non reconnue */
uint8_t cmdNotFound[] = "Command not found\r\n";

/** @brief Message de réponse à la commande WhereisBrian? */
uint8_t brian[] = "Brian is in the kitchen\r\n";

/** @brief Flag indiquant la réception d'un caractère UART */
uint8_t uartRxReceived;

/** @brief Buffer de réception UART */
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];

/** @brief Buffer de transmission UART */
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

/** @brief Buffer pour stocker les données ADC */
uint16_t adcBuffer[BUFFER_SIZE];

/** @brief Buffer stockant la commande en cours de saisie */
char cmdBuffer[CMD_BUFFER_SIZE];

/** @brief Index courant dans le buffer de commande */
int idx_cmd;

/** @brief Tableau des arguments de la commande */
char* argv[MAX_ARGS];

/** @brief Nombre d'arguments de la commande */
int argc = 0;

/** @brief Pointeur utilisé pour le parsing des arguments */
char* token;

/** @brief Flag indiquant qu'une nouvelle commande est prête */
int newCmdReady = 0;

/**
 * @brief Initialise le shell
 * 
 * Cette fonction initialise les buffers et l'UART, puis affiche le message
 * de bienvenue et le prompt initial.
 */
void Shell_Init(void) {
    memset(argv, NULL, MAX_ARGS * sizeof(char*));
    memset(cmdBuffer, NULL, CMD_BUFFER_SIZE * sizeof(char));
    memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE * sizeof(char));
    memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE * sizeof(char));

    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
    HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

/**
 * @brief Boucle principale du shell
 * 
 * Gère la réception des caractères, l'interprétation des commandes et
 * leur exécution. Les commandes supportées sont :
 * - WhereisBrian? : Affiche une phrase humoristique
 * - help : Affiche l'aide
 * - start : Démarre le PWM
 * - stop : Arrête le PWM
 * - current : Affiche la mesure de courant
 * - speed [valeur] : Change la vitesse du moteur
 */
void Shell_Loop(void) {
    if(uartRxReceived) {
        switch(uartRxBuffer[0]) {
            case ASCII_CR: // Nouvelle ligne, instruction à traiter
                HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
                cmdBuffer[idx_cmd] = '\0';
                argc = 0;
                token = strtok(cmdBuffer, " ");
                while(token != NULL) {
                    argv[argc++] = token;
                    token = strtok(NULL, " ");
                }
                idx_cmd = 0;
                newCmdReady = 1;
                break;

            case ASCII_BACK: // Suppression du dernier caractère
                if (idx_cmd > 0) { // Éviter les index négatifs
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

    if(newCmdReady) {
        int commandRecognized = 0; // Indicateur pour vérifier si une commande est reconnue

        if(strcmp(argv[0], "WhereisBrian?") == 0) {
            HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
            commandRecognized = 1;
        }
        else if(strcmp(argv[0], "help") == 0) {
            int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, 
                "Print all available functions here\r\n");
            HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
            commandRecognized = 1;
        }
        else if(strcmp(argv[0], "start") == 0) {
            start_PWM(htim1, TIM_CHANNEL_1);
            commandRecognized = 1;
        }
        else if(strcmp(argv[0], "stop") == 0) {
            stop_PWM(htim1, TIM_CHANNEL_1);
            stop_PWM(htim1, TIM_CHANNEL_2);
            commandRecognized = 1;
        }
        else if(strcmp(argv[0], "current") == 0) {
            /** 
             * @brief Calcul du courant à partir de la mesure ADC
             * Utilise les paramètres suivants :
             * - Sensibilité du capteur : 0.05 V/A
             * - Tension max ADC : 3.3V
             * - Résolution ADC : 4096 points
             * - Offset : 0
             */
            float sensitivity = 0.05;
            float adc_val_max = 4096.0;
            float adc_vcc = 3.3;
            int offset = 0;

            float u_adc = adc_vcc * ((int)(adcBuffer[0]) - offset) / adc_val_max;
            float Imes = (u_adc - 1.65) / sensitivity;

            int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, 
                "Valeur : %d \r\n", adcBuffer[0]);
            HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);

            uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, 
                "ValeurImes : %.2f \r\n", Imes);
            HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
            commandRecognized = 1;
        }
        else if(strcmp(argv[0], "speed") == 0) {
            if(argc > 1) {
                int speedValue = atoi(argv[1]);
                if(speedValue >= 200 && speedValue <= 2000) {
                    // On met le rapport cyclique à la valeur du rapport cyclique renseigné
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, speedValue);
                    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, TIM1->ARR - speedValue);
                    commandRecognized = 1;
                } else {
                    HAL_UART_Transmit(&huart2, (uint8_t *)"Invalid speed value\r\n", 
                        strlen("Invalid speed value\r\n"), HAL_MAX_DELAY);
                }
            }
            else {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Please provide a speed value\r\n", 
                    strlen("Please provide a speed value\r\n"), HAL_MAX_DELAY);
            }
        }

        // Si aucune commande n'est reconnue, envoyer "Command not found"
        if (!commandRecognized) {
            HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
        }

        newCmdReady = 0; // Réinitialiser seulement après exécution ou rejet de la commande
        HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
    }
}

/**
 * @brief Callback appelé lors de la réception d'un caractère UART
 * 
 * Cette fonction est appelée automatiquement lorsqu'un caractère est reçu sur l'UART.
 * Elle met à jour le flag de réception et réactive l'interruption pour le prochain caractère.
 * 
 * @param huart Pointeur vers la structure de l'UART utilisé
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart) {
    uartRxReceived = 1;
    HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}