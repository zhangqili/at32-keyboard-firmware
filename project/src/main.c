/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f402_405_wk_config.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "usbd_user.h"
#include "keyboard.h"
#include "ws2812.h"
#include "analog.h"
#include "rgb.h"
#include "usbd_mtp.h"


/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
volatile uint32_t debug = 0;
static volatile bool init_flag = false;
static uint16_t adc_dma_buffer[18];
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

void usb_dc_low_level_init(uint8_t busid) {
  crm_periph_clock_enable(CRM_OTGHS_PERIPH_CLOCK, TRUE);
  nvic_irq_enable(OTGHS_IRQn, 0, 0);
}
/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config for
     void wk_delay_us(uint32_t delay);
     void wk_delay_ms(uint32_t delay); */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init adc1 function. */
  wk_adc1_init();

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1, 
                        (uint32_t)&ADC1->odt, 
                        DMA1_CHANNEL1_MEMORY_BASE_ADDR, 
                        DMA1_CHANNEL1_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  /* init dma1 channel2 */
  wk_dma1_channel2_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR 
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL2, 
                        (uint32_t)&SPI3->dt, 
                        DMA1_CHANNEL2_MEMORY_BASE_ADDR, 
                        DMA1_CHANNEL2_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL2, TRUE);

  /* init usart1 function. */
  wk_usart1_init();

  /* init usb_otghs1 function. */
  wk_usb_otghs1_init();

  /* init spi3 function. */
  wk_spi3_init();

  /* init exint function. */
  wk_exint_config();

  /* init tmr6 function. */
  wk_tmr6_init();

  /* add user code begin 2 */
  ws2812_init();
  keyboard_init();
  adc_ordinary_software_trigger_enable(ADC1, TRUE);
  rgb_init_flash();
  //wk_delay_ms(100);
  analog_calibrate();
  usb_init(0, OTGHS_BASE);
  init_flag = true;
  g_keyboard_config.enable_report = false;
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
    //printf("tick: %ld, adc: %d, debug: %d\n", g_keyboard_tick, adc_dma_buffer[0], debug);
    keyboard_process();

  #ifdef MTP_ENABLE
    usbd_mtp_task();
  #endif
    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

void main_task(void)
{
  //debug++;
  g_keyboard_tick++;
  if (init_flag)
  {
    keyboard_task();
  }
}

void adc_task(void)
{
  static uint32_t offset;
  g_analog_active_channel++;
  debug++;
  if(g_analog_active_channel > 7) {  /* 43.04us 24kHz*/
    /* one scan completed */
    //p_key->adc_key.is_done = TRUE;
    
    /* clear multiplexer id to 0 start next scan */
    g_analog_active_channel = 0;
    
    //p_key->adc_key.p_values = p_key->adc_key.adc_key_values[p_key->adc_key.id];

    /* modify adc key ping pong buffer id */
    //p_key->adc_key.id ^= 1;
    
  }
  analog_channel_select(g_analog_active_channel);
  adc_ordinary_software_trigger_enable(ADC1, TRUE);
  if(dma_interrupt_flag_get(DMA1_FDT1_FLAG) != RESET)
  {   
    /* add user code begin DMA1_FDT1_FLAG */
    /* handle full data transfer and clear flag */
    for (int i = 0; i < 9; i++) {
      if(offset+i < ANALOG_BUFFER_LENGTH)
      ringbuf_push(&g_adc_ringbufs[offset+i], adc_dma_buffer[9+i]);
    }
    offset+=9;
    dma_flag_clear(DMA1_FDT1_FLAG);
    /* add user code end DMA1_FDT1_FLAG */ 
  }

  if(dma_interrupt_flag_get(DMA1_HDT1_FLAG) != RESET)
  {   
    /* add user code begin DMA1_HDT1_FLAG */
    /* handle half data transfer and clear flag */
    for (int i = 0; i < 9; i++) {
      if(offset+i < ANALOG_BUFFER_LENGTH)
      ringbuf_push(&g_adc_ringbufs[offset+i], adc_dma_buffer[i]);
    }
    offset+=9;
    dma_flag_clear(DMA1_HDT1_FLAG);
    /* add user code end DMA1_HDT1_FLAG */ 
  }
  if(g_analog_active_channel == 0)
  {
    /* next offset to set 0 */
    offset = 0;
  }
}
  /* add user code end 4 */
