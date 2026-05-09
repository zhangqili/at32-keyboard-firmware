/*
 * Copyright (c) 2025 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: MIT
 */
#include "ws2812.h"
//#include "wk_tmr.h"

uint8_t ws2812_buffer[WS2812_BUFFER_LENGTH];

void ws2812_init(void)
{
    for (uint16_t i = 0; i < WS2812_BUFFER_LENGTH; i++)
    {
        ws2812_buffer[i] = NONE_PULSE;
    }
    //tmr_output_enable(TMR3, TRUE);
    //tmr_counter_enable(TMR3, TRUE);
}

void ws2812_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        ws2812_buffer[WS2812_RESET_LENGTH + index * 24 + i] = (g << i) & (0x80) ? ONE_PULSE : ZERO_PULSE;
        ws2812_buffer[WS2812_RESET_LENGTH + index * 24 + i + 8] = (r << i) & (0x80) ? ONE_PULSE : ZERO_PULSE;
        ws2812_buffer[WS2812_RESET_LENGTH + index * 24 + i + 16] = (b << i) & (0x80) ? ONE_PULSE : ZERO_PULSE;
    }
}
