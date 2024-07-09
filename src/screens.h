// This files define the screen setup and helper functions
#pragma once

#include <stdint.h>

#include "main.h"

bool bound_check_simple(int* x, int* y, int dx, int dy, int output);

bool set_screens_info(uint8_t, bool);

extern const screens_info_t screens_info_array[];

static inline int x_coord_A_to_abs(const screens_info_t* screens_info, int x) { 
    return (x * screens_info->width_A / MAX_SCREEN_COORD) + screens_info->x_offset_A;
}

static inline int y_coord_A_to_abs(const screens_info_t* screens_info, int y) { 
    return y * screens_info->height_A / MAX_SCREEN_COORD + screens_info->y_offset_A;
}

static inline int x_coord_abs_to_A(const screens_info_t* screens_info, int x) { 
    return (x - screens_info->x_offset_A) * MAX_SCREEN_COORD / screens_info->width_A;
}

static inline int y_coord_abs_to_A(const screens_info_t* screens_info, int y) { 
    return (y - screens_info->y_offset_A) * MAX_SCREEN_COORD / screens_info->height_A;
}

static inline int x_coord_B_to_abs(const screens_info_t* screens_info, int x) { 
    return (x * screens_info->width_B / MAX_SCREEN_COORD) + screens_info->x_offset_B;
}

static inline int y_coord_B_to_abs(const screens_info_t* screens_info, int y) { 
    return y * screens_info->height_B / MAX_SCREEN_COORD + screens_info->y_offset_B;
}

static inline int x_coord_abs_to_B(const screens_info_t* screens_info, int x) { 
    return (x - screens_info->x_offset_B) * MAX_SCREEN_COORD / screens_info->width_B;
}

static inline int y_coord_abs_to_B(const screens_info_t* screens_info, int y) { 
    return (y - screens_info->y_offset_B) * MAX_SCREEN_COORD / screens_info->height_B;
}

static inline int current_x_coord_A_to_abs(int x) { 
    return x_coord_A_to_abs(global_state.current_screens, x);
}

static inline int current_y_coord_A_to_abs(int y) { 
    return y_coord_A_to_abs(global_state.current_screens, y);
}

static inline int current_x_coord_abs_to_A(int x) { 
    return x_coord_abs_to_A(global_state.current_screens, x);
}

static inline int current_y_coord_abs_to_A(int y) { 
    return y_coord_abs_to_A(global_state.current_screens, y);
}

static inline int current_x_coord_B_to_abs(int x) { 
    return x_coord_B_to_abs(global_state.current_screens, x);
}

static inline int current_y_coord_B_to_abs(int y) { 
    return y_coord_B_to_abs(global_state.current_screens, y);
}

static inline int current_x_coord_abs_to_B(int x) { 
    return x_coord_abs_to_B(global_state.current_screens, x);
}

static inline int current_y_coord_abs_to_B(int y) { 
    return y_coord_abs_to_B(global_state.current_screens, y);
}

// Check if coord is on screen
static inline bool bound_check(int* x, int* y, int dx, int dy, int output) {
    return global_state.current_screens->bound_check(x, y, dx, dy, output);
}

static inline int get_x_speed() {
    if (global_state.active_output == OUTPUT_A) {
        return MOUSE_BASE_SPEED / global_state.current_screens->width_A;
    }
    if (global_state.active_output == OUTPUT_B) {
        return MOUSE_BASE_SPEED / global_state.current_screens->width_B;
    }
    return 16;
}

static inline int get_y_speed() {
    if (global_state.active_output == OUTPUT_A) {
        return MOUSE_BASE_SPEED / global_state.current_screens->height_A;
    }
    if (global_state.active_output == OUTPUT_B) {
        return MOUSE_BASE_SPEED / global_state.current_screens->height_B;
    }
    return 16;
}