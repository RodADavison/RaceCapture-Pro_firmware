/*
 * GPIO.h
 *
 *  Created on: Feb 7, 2014
 *      Author: brent
 */

#ifndef GPIO_H_
#define GPIO_H_
#include "loggerConfig.h"

int GPIO_init(LoggerConfig *loggerConfig);

unsigned int GPIO_get(unsigned int port);
void GPIO_set(unsigned int port, unsigned int state);
int GPIO_is_SD_card_present(void);
int GPIO_is_SD_card_writable(void);
int GPIO_is_button_pressed(void);


#endif /* GPIO_H_ */
