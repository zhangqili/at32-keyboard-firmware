/*
 * Copyright (c) 2025 Zhangqi Li (@zhangqili)
 *
 * SPDX-License-Identifier: MIT
 */
#include "ws2812.h"
#include "at32f402_405.h"
#include "wk_system.h"
//#include "wk_tmr.h"
#include <string.h>

#if WS2812_INTERFACE == WS2812_USE_SPI
#define WS2812_BUFFER_LENGTH    ((WS2812_LED_NUM * 3 * 4) + WS2812_RESET_LENGTH)
#endif

uint8_t ws2812_buffer[WS2812_BUFFER_LENGTH];

void ws2812_init(void)
{
    for (uint16_t i = 0; i < WS2812_BUFFER_LENGTH; i++)
    {
        ws2812_buffer[i] = NONE_PULSE;
    }
    dma_channel_enable(DMA1_CHANNEL2, FALSE);
    DMA1_CHANNEL2->dtcnt = WS2812_BUFFER_LENGTH;
    DMA1_CHANNEL2->paddr = (uint32_t)&SPI3->dt;
    DMA1_CHANNEL2->maddr = (uint32_t)ws2812_buffer;
    dma_channel_enable(DMA1_CHANNEL2, TRUE);
    //tmr_output_enable(TMR3, TRUE);
    //tmr_counter_enable(TMR3, TRUE);
}

int ws2812_flush(void)
{
    if (dma_data_number_get(DMA1_CHANNEL2) != 0)
    {
        return 1; 
    }

    // 2. 关闭 DMA 通道以装载新的传输参数
    dma_channel_enable(DMA1_CHANNEL2, FALSE);
    dma_flag_clear(DMA1_FDT2_FLAG | DMA1_HDT2_FLAG);
    dma_data_number_set(DMA1_CHANNEL2, WS2812_BUFFER_LENGTH);
    // 4. 开启 DMA 发送
    dma_channel_enable(DMA1_CHANNEL2, TRUE);

    return 0;
}

void ws2812_set(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
if (index >= WS2812_LED_NUM) return;

#if WS2812_INTERFACE == WS2812_USE_SPI
    uint8_t *p = &ws2812_buffer[index * 12];
    uint8_t color_bytes[3] = {g, r, b}; // WS2812的数据顺序是 GRB
    for (int i = 0; i < 3; i++) {
        uint8_t val = color_bytes[i];
        for (int bit = 7; bit >= 0; bit -= 2) {
            uint8_t spi_byte = 0;
            if (val & (1 << bit)) spi_byte |= (WS2812_CODE_1 << 4);
            else                  spi_byte |= (WS2812_CODE_0 << 4);
            if (val & (1 << (bit - 1))) spi_byte |= WS2812_CODE_1;
            else                        spi_byte |= WS2812_CODE_0;
            *p++ = spi_byte;
        }
    }
#endif
}

void ws2812_demo(void)
{
    uint32_t step = 0;

    while (1) {
        memset(ws2812_buffer, 0, sizeof(ws2812_buffer)); 
        
        uint32_t active_led = step % WS2812_LED_NUM;
        for (int i = 0; i < WS2812_LED_NUM; i++)
        {
            ws2812_set(i, 0, 0, 0); 
        }
        
        // 设当前灯为白色
        ws2812_set(active_led, 0xFF, 0xFF, 0xFF); 
        step++;

        // 触发SPI DMA发送
        ws2812_flush();

        // 使用AT32库自带的延时函数
        wk_delay_ms(50); 
    }
}
