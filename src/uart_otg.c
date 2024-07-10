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

static inline void forwarder_handle_sending_state(device_t *state) {
    uart_write_blocking(SERIAL_UART, state->forwarder_data, state->forwarder_data_size);
    // Reset buffer and prepare to receive data
    state->forwarder_data_size = 0;
    state->forwarder_expected_reply_length = 0;
    state->forwarder_state = FWD_READING_HEADER;
}

static inline void forwarder_handle_reading_header_state(device_t* state) {
    while (uart_is_readable(SERIAL_UART)) {
        state->forwarder_data[state->forwarder_data_size] = uart_getc(SERIAL_UART);
        if (state->forwarder_data_size == 0) {
            if (state->forwarder_data[0] == START1)
                state->forwarder_data_size ++;
            continue;
        }
        if (state->forwarder_data_size == 1) {
            if (state->forwarder_data[1] == START2)
                state->forwarder_data_size ++;
            else
                state->forwarder_data_size = 0;
            continue;
        }
        if (state->forwarder_data_size == 2) {
            state->forwarder_data_size ++;
            continue;
        }
        if (state->forwarder_data_size == 3) {
            state->forwarder_expected_reply_length = (
                state->forwarder_data[2] |
                (state->forwarder_data[3] << 8)
            );
            state->forwarder_data_size = 0;
            state->forwarder_state = FWD_RECEIVING;
        }
    }

}


static inline void forwarder_handle_receiving_state(device_t *state) {
    while(uart_is_readable(SERIAL_UART) && state->forwarder_data_size < state->forwarder_expected_reply_length) {
        state->forwarder_data[state->forwarder_data_size++] = uart_getc(SERIAL_UART);
    }
    if (state->forwarder_data_size >= state->forwarder_expected_reply_length) {
        state->forwarder_state = FWD_IDLE;
    }
}

void forwarder_task(device_t *state) {
    switch (state->forwarder_signal) {
        // Forwarder is reset into idle state
        case FWD_SIG_RESET:
        state->forwarder_state = FWD_IDLE;
        state->forwarder_signal = FWD_SIG_NONE;
        return;
        // Forwarder is stopped
        case FWD_SIG_STOP:
        state->forwarder_state = FWD_DISABLED;
        state->forwarder_signal = FWD_SIG_NONE;
        return;
        case FWD_SIG_NONE:
        break;
    }
    switch (state->forwarder_state)  {
        // Forwarder is disabled
        case FWD_DISABLED:
        // Forwarder is waiting host to write buffer
        // or has received data and is waiting for host to read out
        case FWD_IDLE:
        return;
        // Forwarder is sending data to the other board
        case FWD_SENDING:
        forwarder_handle_sending_state(state);
        return;
        // Forwarder is reading header from the other board
        case FWD_READING_HEADER:
        forwarder_handle_reading_header_state(state);
        return;
        // Forwarder is receiving data from the other board
        case FWD_RECEIVING:
        forwarder_handle_receiving_state(state);
        return;

    }
}

#endif