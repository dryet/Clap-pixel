/*
 * neopixel.c
 *
 *  Created on: Jan 20, 2024
 *      Author: Petr Opravil
 */

#include "main.h"
#include "neopixel.h"

#include "math.h"

#define TIM_HANDLE htim1
#define TIM_CHANNEL TIM_CHANNEL_1

#define NEOPIXEL_ZERO 25
#define NEOPIXEL_ONE 51
#define NEOPIXEL_NUM 6
#define NEOPIXEL_DMA_BUF_SIZE ((NEOPIXEL_NUM * 24) + 2)

typedef union
{
	struct
	{
		uint8_t b;
		uint8_t r;
		uint8_t g;
	} color;
	uint32_t data;
} PixelRGB_t;

uint32_t dmaBuffer[NEOPIXEL_DMA_BUF_SIZE] = {0}; // Needs to be global, because of volatility

/**
 * @brief  Function that gets rgb input and sets it on a single neopixel led
 * @param  r: Red color value
 * @param  g: Green color value
 * @param  b: Blue color value
 * @retval None
 */
void neopixel_set_color_rgb(uint8_t r, uint8_t g, uint8_t b)
{

	PixelRGB_t pixel = {0};
	uint32_t *pBuff;

	pixel.color.r = r;
	pixel.color.g = g;
	pixel.color.b = b;

	pBuff = dmaBuffer;

	// First element should be zero
	*pBuff = 0;
	pBuff++;

	for (int i = 0; i < NEOPIXEL_NUM; i++)
	{
		for (int j = 23; j >= 0; j--)
		{
			if ((pixel.data >> j) & 0x01)
			{
				*pBuff = NEOPIXEL_ONE;
			}
			else
			{
				*pBuff = NEOPIXEL_ZERO;
			}
			pBuff++;
		}
	}

	dmaBuffer[NEOPIXEL_DMA_BUF_SIZE - 1] = 0; // last element must be 0!

	HAL_TIM_PWM_Start_DMA(&TIM_HANDLE, TIM_CHANNEL, dmaBuffer,
						  NEOPIXEL_DMA_BUF_SIZE);
}

/**
 * @brief  Function that gets hsv input and sets it on a single neopixel led
 * @param  h: Hue of the set color
 * @param  s: Saturation of the set color
 * @param  v: Value of the set color
 * @retval None
 */
void neopixel_set_color_hsv(uint8_t h, uint8_t s, uint8_t v)
{
	float r, g, b;

	// Convert input values to a normalized range
	float hf = h / 255.0 * 360; // Convert hue from [0,255] to [0,360]
	float sf = s / 255.0;		// Convert saturation from [0,255] to [0,1]
	float vf = v / 255.0;		// Convert value from [0,255] to [0,1]

	float h_norm = hf / 360.0; // Normalize hue to [0,1] range
	int i = floor(h_norm * 6);
	float f = h_norm * 6 - i;
	float p = vf * (1 - sf);
	float q = vf * (1 - f * sf);
	float t = vf * (1 - (1 - f) * sf);

	switch (i % 6)
	{
	case 0:
		r = vf, g = t, b = p;
		break;
	case 1:
		r = q, g = vf, b = p;
		break;
	case 2:
		r = p, g = vf, b = t;
		break;
	case 3:
		r = p, g = q, b = vf;
		break;
	case 4:
		r = t, g = p, b = vf;
		break;
	case 5:
		r = vf, g = p, b = q;
		break;
	}

	// Convert to 8-bit integer values
	uint8_t r_out = (uint8_t)(r * 255);
	uint8_t g_out = (uint8_t)(g * 255);
	uint8_t b_out = (uint8_t)(b * 255);

	neopixel_set_color_rgb(r_out, g_out, b_out);
}

/**
 * @brief  PWM Pulse finished callback in non-blocking mode
 * @param  htim TIM handle
 * @retval None
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL); // Stops timer output after the last pulse
}
