#if BOARD_ROLE == PICO_A
#include "main.h"

const host_msg_handler_t host_msg_handler[] = {
    {.type = SET_SCREENS_INFO_HOST_MSG, .handler = host_handle_set_screens_info_msg},
    {.type = FORWARDER_WRITE_HOST_MSG, .handler = host_handle_forwarder_write},
    {.type = FORWARDER_SEND_HOST_MSG, .handler = host_handle_forwarder_send},
    {.type = FORWARDER_READ_HOST_MSG, .handler = host_handle_forwarder_read},
    {.type = FORWARDER_STOP_HOST_MSG, .handler = host_handle_forwarder_stop},
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

void send_host_message_resp(const uint8_t * data, enum host_msg_resp_type_e type, int length) {
    uint8_t buffer[CFG_TUD_HID_EP_BUFSIZE];
    buffer[0] = type;
    memcpy(buffer+1, data, length);
    tud_hid_n_report(ITF_NUM_HID, REPORT_ID_GENERIC, buffer, length + 1);
}

void host_message_resp_status(enum host_msg_resp_type_e status) {
    send_host_message_resp(NULL, status, 0);
}

#endif