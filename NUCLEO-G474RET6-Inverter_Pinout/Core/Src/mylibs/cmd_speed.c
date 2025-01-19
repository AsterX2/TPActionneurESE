/**
 * @file cmd_speed.c
 * @brief Gestion du contrôle de la vitesse moteur par PWM
 * @author MundusEst
 * @date 13 Novembre 2024
 * 
 * Ce fichier contient les fonctions de contrôle du moteur via PWM,
 * permettant de démarrer, arrêter et réguler la vitesse du moteur.
 * Il gère également la mesure du courant via l'ADC.
 */

#include <stdint.h>
#include "tim.h"

/**
 * @brief Valeur maximale du rapport cyclique (90% de la période)
 * Cette valeur limite la puissance maximale fournie au moteur
 */
#define alpha_MAX 1913

/**
 * @brief Valeur moyenne du rapport cyclique (50% de la période)
 * Utilisée comme point de départ lors du démarrage du moteur
 */
#define alpha_MID 1063

/**
 * @brief Délai entre chaque pas de la rampe en millisecondes
 * Permet une accélération progressive du moteur
 */
#define RAMP_DELAY 10

/**
 * @brief Pas d'incrémentation du rapport cyclique
 * Définit la granularité du contrôle de vitesse
 */
#define RAMP_STEP 5

/** @brief Handle de l'ADC utilisé pour la mesure de courant */
extern ADC_HandleTypeDef hadc1;

/**
 * @brief Démarre la génération PWM
 * 
 * Cette fonction configure et démarre la génération PWM sur les deux canaux
 * en mode complémentaire. Elle initialise également les rapports cycliques
 * à 50% pour un démarrage équilibré.
 * 
 * @param htim Handle du timer utilisé pour la génération PWM
 * @param channel Numéro du canal PWM à configurer
 */
void start_PWM(TIM_HandleTypeDef htim, uint32_t channel) {
    // Démarrage du canal PWM principal et de son complémentaire
    HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim, TIM_CHANNEL_2);

    // Configuration du rapport cyclique initial à 50%
    int alpha = alpha_MID;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, alpha);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, TIM1->ARR - alpha);
}

/**
 * @brief Arrête la génération PWM
 * 
 * Cette fonction arrête la génération PWM sur le canal spécifié
 * ainsi que sur sa sortie complémentaire. Elle est utilisée pour
 * arrêter le moteur en toute sécurité.
 * 
 * @param htim Handle du timer utilisé
 * @param channel Numéro du canal PWM à arrêter
 */
void stop_PWM(TIM_HandleTypeDef htim, uint32_t channel) {
    HAL_TIM_PWM_Stop(&htim, channel);
    HAL_TIMEx_PWMN_Stop(&htim, channel);
}

/**
 * @brief Calcul du courant à partir de la mesure ADC (fonction commentée)
 * 
 * Cette fonction calcule le courant consommé par le moteur à partir
 * des mesures ADC. Le calcul prend en compte :
 * - La tension de référence (3.3V)
 * - La résolution ADC (4096 points)
 * - L'offset du capteur
 * - La sensibilité du capteur de courant (0.05V/A)
 * 
 * La formule utilisée est : Imes = (Vadc - 1.65) / 0.05
 * où Vadc = 3.3 * mesure_adc / 4096
 * 
 * @param adc_buf Valeur brute de l'ADC
 * @param adc_res Résolution de l'ADC
 * @return float Valeur du courant calculée en ampères
 */
/*float calc_I(uint32_t adc_buf, int adc_res) {
    (int) adc_buf = (adc_buf)/(adc_res);

    int value_u = (3.3*u)/4096;
    float Imes = (value_u -((1,65))/(0,05));
    return Imes;

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    int value = HAL_ADC_GetValue(&hadc1);
    float Imes = (value-((1,65))/(0,05));
    int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, 
        "Valeur adc lue: %4d \r\n", value);
    HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
    uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, 
        "ValeurImes post calc: %4d \r\n", Imes);
    HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
}*/

/**
 * @brief Modification progressive de la vitesse PWM (fonction commentée)
 * 
 * Cette fonction implémente une rampe de vitesse pour le moteur,
 * permettant des transitions douces entre différentes vitesses.
 * Elle évite les changements brusques qui pourraient endommager
 * le moteur ou la mécanique.
 * 
 * Caractéristiques :
 * - Limite la vitesse maximale
 * - Utilise une rampe progressive
 * - Gère à la fois l'accélération et la décélération
 * 
 * @param htim Handle du timer
 * @param channel Canal PWM à modifier
 * @param target_speed Vitesse cible à atteindre
 */
/*void speed_PWM(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t target_speed) {
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
}*/