/*
 * This file is part of DeskHop (https://github.com/hrvach/deskhop).
 * Copyright (c) 2024 Hrvoje Cavrak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "screens.h"

// TODO: get rid of border/screen handling code here that is no longer needed
/**=================================================== *
 * ============  Hotkey Handler Routines  ============ *
 * =================================================== */

/* This is the main hotkey for switching outputs */
void output_toggle_hotkey_handler(device_t *state, hid_keyboard_report_t *report) {
    /* If switching explicitly disabled, return immediately */
    if (state->switch_lock)
        return;

    state->active_output ^= 1;
    switch_output(state, state->active_output);
};


/* This key combo puts board A in firmware upgrade mode */
void fw_upgrade_hotkey_handler_A(device_t *state, hid_keyboard_report_t *report) {
#if BOARD_ROLE == PICO_A
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
#endif
#if BOARD_ROLE == PICO_B
    send_value(ENABLE, FIRMWARE_UPGRADE_MSG);
#endif
};

/* This key combo puts board B in firmware upgrade mode */
void fw_upgrade_hotkey_handler_B(device_t *state, hid_keyboard_report_t *report) {
#if BOARD_ROLE == PICO_B
    reboot_to_serial_bootloader();
#endif
#if BOARD_ROLE == PICO_A
    send_value(ENABLE, FIRMWARE_UPGRADE_MSG);
#endif
};

/* This key combo prevents mouse from switching outputs */
void switchlock_hotkey_handler(device_t *state, hid_keyboard_report_t *report) {
    state->switch_lock ^= 1;
    send_value(state->switch_lock, SWITCH_LOCK_MSG);
}

/* This key combo locks both outputs simultaneously */
void screenlock_hotkey_handler(device_t *state, hid_keyboard_report_t *report) {
    hid_keyboard_report_t lock_report = {0}, release_keys = {0};

    for (int out = 0; out < NUM_SCREENS; out++) {
        switch (state->config.output[out].os) {
            case WINDOWS:
                lock_report.modifier   = KEYBOARD_MODIFIER_LEFTGUI;
                lock_report.keycode[0] = HID_KEY_L;
                break;
            case LINUX:
                lock_report.modifier   = KEYBOARD_MODIFIER_LEFTGUI| KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT;
                lock_report.keycode[0] = HID_KEY_L;
                break;
            case MACOS:
                lock_report.modifier   = KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI;
                lock_report.keycode[0] = HID_KEY_Q;
                break;
            default:
                break;
        }

        if (BOARD_ROLE == out) {
            queue_kbd_report(&lock_report, state);
            release_all_keys(state);
        } else {
            send_packet((uint8_t *)&lock_report, KEYBOARD_REPORT_MSG, KBD_REPORT_LENGTH);
            send_packet((uint8_t *)&release_keys, KEYBOARD_REPORT_MSG, KBD_REPORT_LENGTH);
        }
    }
}

/* When pressed, toggles the current mouse zoom mode state */
void mouse_zoom_hotkey_handler(device_t *state, hid_keyboard_report_t *report) {
    state->mouse_zoom ^= 1;
    send_value(state->mouse_zoom, MOUSE_ZOOM_MSG);
};

/**==================================================== *
 * ==========  UART Message Handling Routines  ======== *
 * ==================================================== */

/* Function handles received keypresses from the other board */
void handle_keyboard_uart_msg(uart_packet_t *packet, device_t *state) {
    queue_kbd_report((hid_keyboard_report_t *)packet->data, state);
    state->last_activity[BOARD_ROLE] = time_us_64();
}

/* Function handles received mouse moves from the other board */
void handle_mouse_abs_uart_msg(uart_packet_t *packet, device_t *state) {
    mouse_report_t *mouse_report = (mouse_report_t *)packet->data;
    queue_mouse_report(mouse_report, state);

    state->last_activity[BOARD_ROLE] = time_us_64();
}

/* Function handles request to switch output  */
void handle_output_select_msg(uart_packet_t *packet, device_t *state) {
    state->active_output = packet->data[0];
    if (state->tud_connected)
        release_all_keys(state);

    restore_leds(state);
}

/* On firmware upgrade message, reboot into the BOOTSEL fw upgrade mode */
void handle_fw_upgrade_msg(uart_packet_t *packet, device_t *state) {
#if BOARD_ROLE == PICO_A
    reset_usb_boot(1 << PICO_DEFAULT_LED_PIN, 0);
#endif
#if BOARD_ROLE == PICO_B
    reboot_to_serial_bootloader();
#endif
}

/* Comply with request to turn mouse zoom mode on/off  */
void handle_mouse_zoom_msg(uart_packet_t *packet, device_t *state) {
    state->mouse_zoom = packet->data[0];
}

/* Process request to update keyboard LEDs */
void handle_set_report_msg(uart_packet_t *packet, device_t *state) {
    state->keyboard_leds[BOARD_ROLE] = packet->data[0];
    restore_leds(state);
}

/* Process request to block mouse from switching, update internal state */
void handle_switch_lock_msg(uart_packet_t *packet, device_t *state) {
    state->switch_lock = packet->data[0];
}

/* When this message is received, flash the locally attached LED to verify serial comms */
void handle_flash_led_msg(uart_packet_t *packet, device_t *state) {
    blink_led(state);
}

/* When this message is received, wipe the local flash config */
void handle_wipe_config_msg(uart_packet_t *packet, device_t *state) {
    wipe_config();
    load_config(state);
}

/* Process consumer control keyboard message. Send immediately, w/o queing */
void handle_consumer_control_msg(uart_packet_t *packet, device_t *state) {
    tud_hid_n_report(0, REPORT_ID_CONSUMER, &packet->data[0], CONSUMER_CONTROL_LENGTH);
}

// handling synced screens info
void handle_screens_info_msg(uart_packet_t *packet, device_t *state) {
    uint8_t layout_index = packet->data[0];
    set_screens_info(layout_index, false);

    restore_leds(state);
}

#ifdef DH_DEBUG
#if BOARD_ROLE == PICO_A
void handle_debug_msg(uart_packet_t * packet, device_t * state) {
    char buffer[9] = {0};
    memcpy(buffer, packet->data, 8);
    int string_len = strlen(buffer);
    tud_cdc_n_write(0, buffer, string_len);
    tud_cdc_write_flush();
}
#endif
#endif

#if BOARD_ROLE == PICO_A
// Host side messaging handlers

// handling set screens info command from host
void host_handle_set_screens_info_msg(uint8_t const* data, uint16_t length){
    uint8_t layout_index = data[0];
    set_screens_info(layout_index, true);

    restore_leds(&global_state);
}
#endif

/**==================================================== *
 * ==============  Output Switch Routines  ============ *
 * ==================================================== */

/* Update output variable, set LED on/off and notify the other board so they are in sync. */
void switch_output(device_t *state, uint8_t new_output) {
    state->active_output = new_output;
    restore_leds(state);
    send_value(new_output, OUTPUT_SELECT_MSG);

    /* If we were holding a key down and drag the mouse to another screen, the key gets stuck.
       Changing outputs = no more keypresses on the previous system. */
    release_all_keys(state);
}
