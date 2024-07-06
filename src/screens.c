#include "screens.h"

bool bound_check_simple(int* x, int* y, int dx, int dy, int output) {
    bool in_screen = true;
    int new_x = *x + dx;
    int new_y = *y + dy;
    if (new_x < MIN_SCREEN_COORD) {
        in_screen = false;
        new_x = MIN_SCREEN_COORD;
    }
    if (new_x > MAX_SCREEN_COORD) {
        in_screen = false;
        new_x = MAX_SCREEN_COORD;
    }
    if (new_y < MIN_SCREEN_COORD) {
        in_screen = false;
        new_y = MIN_SCREEN_COORD;
    }
    if (new_y > MAX_SCREEN_COORD) {
        in_screen = false;
        new_y = MAX_SCREEN_COORD;
    }
    *x = new_x;
    *y = new_y;
    return in_screen;
}

// 1920 * MAX_SCREEN_COORD / (1920 + 1080)
#define SCREEN_X_ALT_BOUND 20970
// (1440 + 1080 - 1920) * MAX_SCREEN_COORD / (1440 + 1080)
#define SCREEN_Y_ALT_BOUND_1 7801
// 1080 * MAX_SCREEN_COORD / (1440 + 1080)
#define SCREEN_Y_ALT_BOUND_2 14043

static bool bound_check_screen(int* x, int* y, int dx, int dy, int output) {
    if (output == OUTPUT_B) return bound_check_simple(x, y, dx, dy, output);
    bool in_screen = true;
    int new_x = *x + dx;
    int new_y = *y + dy;
    if (output == OUTPUT_A) {
        if (new_x < MIN_SCREEN_COORD) {
            in_screen = false;
            new_x = MIN_SCREEN_COORD;
        }
        if (new_x > MAX_SCREEN_COORD) {
            in_screen = false;
            new_x = MAX_SCREEN_COORD;
        }
        if (new_y < MIN_SCREEN_COORD) {
            in_screen = false;
            new_y = MIN_SCREEN_COORD;
        }
        if (new_y > MAX_SCREEN_COORD) {
            in_screen = false;
            new_y = MAX_SCREEN_COORD;
        }

        if (new_x > SCREEN_X_ALT_BOUND && *y < SCREEN_Y_ALT_BOUND_1) {
            in_screen = false;
            new_x = SCREEN_X_ALT_BOUND;
        }

        if (new_y < SCREEN_Y_ALT_BOUND_1 && *x > SCREEN_X_ALT_BOUND) {
            in_screen = false;
            new_y = SCREEN_Y_ALT_BOUND_1;
        }

        if (new_y > SCREEN_Y_ALT_BOUND_2 && *x < SCREEN_X_ALT_BOUND) {
            in_screen = false;
            new_y = SCREEN_Y_ALT_BOUND_2;
        }
        
        if (new_x < SCREEN_X_ALT_BOUND && *y > SCREEN_Y_ALT_BOUND_2) {
            in_screen = false;
            new_x = SCREEN_X_ALT_BOUND;

        }
    }
    *x = new_x;
    *y = new_y;
    return in_screen;
}


const screens_info_t screens_info_array[] = {{
    .height_A = 1440 + 1080,
    .width_A = 1920 + 1080,
    .x_offset_A = 0,
    .y_offset_A = 0,
    .height_B = 1440,
    .width_B = 2560,
    .x_offset_B = 1920 - 2560,
    .y_offset_B = 1080 - 0,
    .bound_check = &bound_check_screen
}};


// set screens info, always reset mouse position to 0, 0 as well
void set_screens_info(uint8_t index, bool sync) {
    if (index >= ARRAY_SIZE(screens_info_array)) return;
    global_state.current_screens = screens_info_array + index;
    global_state.mouse_x = 0;
    global_state.mouse_y = 0;
    if (sync) send_value(index, SCREENS_INFO_MSG);
}
