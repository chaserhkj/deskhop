#include "main.h"
#if BOARD_ROLE == PICO_B

void reboot_to_serial_bootloader() {
    // Write watchdog bootloader flag
    watchdog_hw->scratch[5] = BOOTLOADER_ENTRY_MAGIC;
    watchdog_hw->scratch[6] = ~BOOTLOADER_ENTRY_MAGIC;
    // Watchdog reset
    watchdog_reboot(0, 0, REBOOT_DELAY_MICRO_S);
}

#endif