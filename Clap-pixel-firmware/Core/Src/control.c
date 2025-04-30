/*
 * control.c
 *
 *  Created on: Apr 30, 2025
 *      Author: Petr Opravil
 */

#include <stdio.h>
#include "adc.h"
#include "usart.h"
#include "control.h"
#include "neopixel.h"

#define LED_COLOR_DELAY 150
#define LED_SATURATION_VALUE 200

#define LED_HIGH_VALUE 200
#define LED_HIGH_THRESHOLD 20
#define LED_LOW_THRESHOLD 8
#define LED_BUFFER_TIME_MS 30
#define LED_LOW_TIME_MS 140
#define LED_RESET_TIME_MS 750
#define LED_DEBOUNCE_TIME_MS 1000

enum clap_state_e
{
  WAIT_FIRST,
  BUFFER,
  STAY_LOW,
  WAIT_SECOND,
  TURN_ON
};
enum clap_state_e clap_state;
uint16_t cnt_timer = 0;

uint32_t mic_raw_data;
uint16_t mic_prev_data;

uint16_t led_value = 0;

uint16_t hue;

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

void Control_Init () {
    hue = 0;
    HAL_ADC_Start_DMA(&hadc1, &mic_raw_data, 1);
}

void Control_Loop () {
    hue += 2;
    if (hue > 255)
    {
      hue = 0;
    }
    neopixel_set_color_hsv(hue, LED_SATURATION_VALUE, led_value);
    HAL_Delay(LED_COLOR_DELAY);
}

void ADC_update_on_tick(void)
{
  cnt_timer--;

  if (!cnt_timer)
  {
    switch (clap_state)
    {
    case BUFFER:
      clap_state = STAY_LOW;
      cnt_timer = LED_LOW_TIME_MS;
      break;
    case STAY_LOW:
      clap_state = WAIT_SECOND;
      cnt_timer = LED_RESET_TIME_MS;
      break;
    case WAIT_SECOND:
      clap_state = WAIT_FIRST;
      break;
    case TURN_ON:
      clap_state = WAIT_FIRST;
      HAL_ADC_Start_DMA(&hadc1, &mic_raw_data, 1);
      break;
    default:
      break;
    }
  }
}

/**
 * @brief  End Of Sampling callback in non-blocking mode.
 * @param hadc ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  UNUSED(hadc);

  uint16_t abs_value;
  int16_t difference;

  difference = mic_prev_data - mic_raw_data;
  mic_prev_data = mic_raw_data;
  if (difference < 0)
  {
    abs_value = (difference ^ 0xFFFF) + 1;
  }
  else
  {
    abs_value = difference;
  }
  assert_param(abs_value < 4096);
  printf("%u\n\r", abs_value);

  switch (clap_state)
  {
  case WAIT_FIRST:
    if (abs_value > LED_HIGH_THRESHOLD)
    {
        clap_state = BUFFER;
        cnt_timer = LED_BUFFER_TIME_MS;
    }
    break;
  case STAY_LOW:
    if (abs_value > LED_LOW_THRESHOLD)
    {
        clap_state = WAIT_FIRST;
    }
    break;
  case WAIT_SECOND:
    if (abs_value > LED_HIGH_THRESHOLD)
    {
        led_value = led_value ? 0 : LED_HIGH_VALUE;
        clap_state = TURN_ON;
        cnt_timer = LED_DEBOUNCE_TIME_MS;
        mic_raw_data = 0;
    }
    break;
  default:
    break;
  }

  if (clap_state != TURN_ON)
  {
    HAL_ADC_Start_DMA(&hadc1, &mic_raw_data, 1);
  }
}

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

  return ch;
}
