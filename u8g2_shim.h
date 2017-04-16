/*
 * u8g2_shim.h
 *
 *  Created on: 14 Apr 2017
 *      Author: Dominic
 */

#ifndef U8G2_SHIM_H_
#define U8G2_SHIM_H_

#include "./u8g2/csrc/u8g2.h"
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void delay_init();
void delay_microseconds(uint32_t delay);

#endif /* U8G2_SHIM_H_ */
