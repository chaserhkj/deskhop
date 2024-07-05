#include "screens.h"

uint8_t bound_check_simple(int x, int y, int output) {
    uint8_t result = 0;
    if (x < MIN_SCREEN_COORD) {
        result = result | X_OUT;
    }
    if (x > MAX_SCREEN_COORD) {
        result = result | X_OUT;
        result = result | X_OVERFLOW;
    }
    if (y < MIN_SCREEN_COORD) {
        result = result | Y_OUT;
    }
    if (y > MAX_SCREEN_COORD) {
        result = result | Y_OUT;
        result = result | Y_OVERFLOW;
    }
    return result;
}

static uint8_t bound_check_screen(int x, int y, int output) {
    if (output == OUTPUT_B) return bound_check_simple(x, y, output);
    int result = 0;
    if (output == OUTPUT_A) {
        int abs_x = x_coord_A_to_abs(screens_info_array, x);
        int abs_y = y_coord_A_to_abs(screens_info_array, y);
        if (abs_x < 0 ) {
            result = result | X_OUT;
        }
        if (abs_x > 1920 && abs_y <= 1440 + 1080 - 1920) {
            result = result | X_OUT;
            result = result | X_OVERFLOW;
        }
        if (abs_x > 1920 + 1080 && abs_y > 1440 + 1080 - 1920) {
            result = result | X_OUT;
            result = result | X_OVERFLOW;
        }
        if (abs_y < 0 && abs_x <= 1920) {
            result = result | Y_OUT;
        }
        if (abs_y < 1440 + 1080 - 1920 && abs_x > 1920) {
            result = result | Y_OUT;
        }
        if (abs_y > 1080 && abs_x <= 1920) {
            result = result | Y_OUT;
            result = result | Y_OVERFLOW;
        }
        if (abs_y > 1440 + 1080 && abs_x > 1920) {
            result = result | Y_OUT;
            result = result | Y_OVERFLOW;
        }
    }
    return result;
}


const screens_info_t screens_info_array[] = {{
    .height_A = 1440 + 1080,
    .width_A = 1920 + 1080,
    .x_offset_A = 0,
    .y_offset_A = 0,
    .height_B = 1440,
    .width_B = 2048,
    .x_offset_B = 1920 - 2560,
    .y_offset_B = 1080 - 0,
    .bound_check = &bound_check_screen
}};

