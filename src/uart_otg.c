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

// Simple forwarder for UART OTG protocol
// OTG protocol uses variable packet length, but
// we do not care about the content of the payload
// So we only figure out the length here

