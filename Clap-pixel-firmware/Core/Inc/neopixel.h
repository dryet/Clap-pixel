/*
 * neopixel.h
 *
 *  Created on: Jan 20, 2024
 *      Author: Petr Opravil
 */

#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

#include "main.h"

void neopixel_set_color_rgb(uint8_t r, uint8_t g, uint8_t b);
void neopixel_set_color_hsv(uint8_t h, uint8_t s, uint8_t v);

#endif /* INC_NEOPIXEL_H_ */
