#include "main.h"
#if BOARD_ROLE == PICO_B

void reboot_to_serial_bootloader() {
    // Disable watchdog timer to avoid race condition
    hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
    // Write watchdog bootloader flag
    watchdog_hw->scratch[5] = BOOTLOADER_ENTRY_MAGIC;
    watchdog_hw->scratch[6] = ~BOOTLOADER_ENTRY_MAGIC;
    // Watchdog reset
    watchdog_reboot(0, 0, 0);
}

#endif

#if BOARD_ROLE == PICO_A
// Simple forwarder for UART OTG protocol

static inline void forwarder_handle_send_state(device_t *state) {
    uart_write_blocking(SERIAL_UART, state->forwarder_out_data, state->forwarder_out_data_size);
    // Reset in buffer and prepare to receive data
    // Clear in buffer
    state->forwarder_in_data_read = 0;
    state->forwarder_in_data_written = 0;
    state->forwarder_state = FWD_RECEIVING;
}



static inline void forwarder_handle_receiving_state(device_t *state) {
    while(uart_is_readable(SERIAL_UART)) {
        int new_index = (state->forwarder_in_data_written+1)%FORWARDER_BUF_SIZE;

        if (new_index == state->forwarder_in_data_read) {
            // Buffer overrun, stop reading immediately
            state->forwarder_state = FWD_OVERRAN;
            return;
        }

        state->forwarder_in_data[state->forwarder_in_data_written]  
            = uart_getc(SERIAL_UART);
        state->forwarder_in_data_written = new_index;
    }
}

void forwarder_task(device_t *state) {
    switch (state->forwarder_state)  {
        // Forwarder is disabled
        case FWD_DISABLED:
        // Forwarder is waiting host to write out-buffer
        case FWD_IDLE:
        // Buffer overran in in-buffer
        case FWD_OVERRAN:
        return;
        // Forwarder is sending data to the other board
        case FWD_SEND:
        forwarder_handle_send_state(state);
        return;
        // Forwarder is receiving data from the other board
        case FWD_RECEIVING:
        forwarder_handle_receiving_state(state);
        return;
    }
}

#endif