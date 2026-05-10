#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "usb_dwc2_param.h"

#if __has_include("at32f402_405.h")
#include "at32f402_405.h"
#endif

#include "usb_descriptor.h"

#define DWC2_FS_TOTAL_FIFO_WORDS 320U
#define DWC2_HS_TOTAL_FIFO_WORDS 1008U
#define DWC2_FS_MIN_RX_FIFO_WORDS 47U
#define DWC2_HS_MIN_RX_FIFO_WORDS 159U
#define DWC2_MIN_TX_FIFO_WORDS 16U
#define EP0_MAX_PACKET_SIZE 64U

static void clear_tx_fifos(struct usb_dwc2_user_fifo_config *config)
{
    for (uint8_t i = 0; i < MAX_EPS_CHANNELS; i++) {
        config->device_tx_fifo_size[i] = 0;
    }
}

static uint16_t tx_fifo_words(uint16_t packet_size)
{
    uint16_t words = (uint16_t)((packet_size + 3U) / 4U);

    return words < DWC2_MIN_TX_FIFO_WORDS ? DWC2_MIN_TX_FIFO_WORDS : words;
}

static void reserve_in_fifo(struct usb_dwc2_user_fifo_config *config, uint8_t epnum, uint16_t packet_size)
{
    uint16_t words;

    if (epnum >= MAX_EPS_CHANNELS) {
        return;
    }

    words = tx_fifo_words(packet_size);
    if (config->device_tx_fifo_size[epnum] < words) {
        config->device_tx_fifo_size[epnum] = words;
    }
}

static uint16_t sum_tx_fifos(const struct usb_dwc2_user_fifo_config *config)
{
    uint16_t words = 0;

    for (uint8_t i = 0; i < MAX_EPS_CHANNELS; i++) {
        words = (uint16_t)(words + config->device_tx_fifo_size[i]);
    }

    return words;
}

static void reserve_descriptor_fifos(struct usb_dwc2_user_fifo_config *config)
{
    clear_tx_fifos(config);

    reserve_in_fifo(config, 0, EP0_MAX_PACKET_SIZE);

#ifndef KEYBOARD_SHARED_EP
    reserve_in_fifo(config, KEYBOARD_IN_EPNUM, KEYBOARD_EPSIZE);
#endif

#if defined(MOUSE_ENABLE) && !defined(MOUSE_SHARED_EP)
    reserve_in_fifo(config, MOUSE_IN_EPNUM, MOUSE_EPSIZE);
#endif

#ifdef RAW_ENABLE
    reserve_in_fifo(config, RAW_IN_EPNUM, RAW_EPSIZE);
#endif

#ifdef SHARED_EP_ENABLE
    reserve_in_fifo(config, SHARED_IN_EPNUM, SHARED_EPSIZE);
#endif

#ifdef CONSOLE_ENABLE
    reserve_in_fifo(config, CONSOLE_IN_EPNUM, CONSOLE_EPSIZE);
#endif

#ifdef MIDI_ENABLE
    reserve_in_fifo(config, MIDI_STREAM_IN_EPNUM, MIDI_STREAM_EPSIZE);
#endif

#ifdef VIRTSER_ENABLE
    reserve_in_fifo(config, CDC_NOTIFICATION_EPNUM, CDC_NOTIFICATION_EPSIZE);
    reserve_in_fifo(config, CDC_IN_EPNUM, CDC_EPSIZE);
#endif

#if defined(JOYSTICK_ENABLE) && !defined(JOYSTICK_SHARED_EP)
    reserve_in_fifo(config, JOYSTICK_IN_EPNUM, JOYSTICK_EPSIZE);
#endif

#if defined(DIGITIZER_ENABLE) && !defined(DIGITIZER_SHARED_EP)
    reserve_in_fifo(config, DIGITIZER_IN_EPNUM, DIGITIZER_EPSIZE);
#endif

#ifdef MTP_ENABLE
    reserve_in_fifo(config, MTP_EVT_EPNUM, MTP_EVENT_EPSIZE);
    reserve_in_fifo(config, MTP_IN_EPNUM, MTP_DATA_EPSIZE);
#endif

#ifdef GAMEPAD_ENABLE
    reserve_in_fifo(config, XINPUT_IN_EPNUM, XINPUT_EPSIZE);
#endif
}

static void use_remaining_fifo_for_rx(struct usb_dwc2_user_fifo_config *config, uint16_t total_words, uint16_t min_rx_words)
{
    uint16_t tx_words = sum_tx_fifos(config);

    if ((uint32_t)tx_words + min_rx_words > total_words) {
        config->device_rx_fifo_size = min_rx_words;
        return;
    }

    config->device_rx_fifo_size = total_words - tx_words;
}

void dwc2_get_user_fifo_config(uint32_t reg_base, struct usb_dwc2_user_fifo_config *config)
{
    if (config == NULL) {
        return;
    }

    reserve_descriptor_fifos(config);

#ifdef OTGHS_BASE
    if (reg_base == OTGHS_BASE) {
        use_remaining_fifo_for_rx(config, DWC2_HS_TOTAL_FIFO_WORDS, DWC2_HS_MIN_RX_FIFO_WORDS);
        return;
    }
#else
    (void)reg_base;
#endif

    use_remaining_fifo_for_rx(config, DWC2_FS_TOTAL_FIFO_WORDS, DWC2_FS_MIN_RX_FIFO_WORDS);
}
