#if BOARD_ROLE == PICO_A
#include "main.h"

const host_msg_handler_t host_msg_handler[] = {
    {.type = SET_SCREENS_INFO_HOST_MSG, .handler = host_handle_set_screens_info_msg},
};

void process_host_message(uint8_t const* data, uint16_t length) {
    if (length <= 0) return;
    uint8_t type = data[0];
    for (int i = 0; i < ARRAY_SIZE(host_msg_handler); i++) {
        if (host_msg_handler[i].type == type) {
            host_msg_handler[i].handler(data+1, length-1);
            return;
        }
    }
}

void send_host_message(const uint8_t * data, enum host_msg_type_e type, int length) {
    uint8_t buffer[CFG_TUD_HID_EP_BUFSIZE];
    buffer[0] = type;
    memcpy(buffer+1, data, length);
    tud_hid_n_report(ITF_NUM_HID, REPORT_ID_GENERIC, buffer, length + 1);
}
#endif