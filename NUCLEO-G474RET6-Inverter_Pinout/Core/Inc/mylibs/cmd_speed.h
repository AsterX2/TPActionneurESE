/*
 * cmd_speed.h
 *
 *  Created on: Nov 13, 2024
 *      Author: MundusEst
 */
#include <stdint.h>
#include "tim.h"

#ifndef INC_MYLIBS_CMD_SPEED_H_
#define INC_MYLIBS_CMD_SPEED_H_



void start_PWM(TIM_HandleTypeDef htim,uint32_t);
void stop_PWM(TIM_HandleTypeDef,uint32_t);




#endif /* INC_MYLIBS_CMD_SPEED_H_ */
