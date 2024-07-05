// This files define the screen setup and helper functions
#pragma once

#include <stdint.h>

#include "main.h"

typedef struct {
    uint32_t width_A;
    uint32_t width_B;
    uint32_t height_A;
    uint32_t height_B;
    int32_t x_offset_A;
    int32_t y_offset_A;
    int32_t x_offset_B;
    int32_t y_offset_B;
} screens_info_t;

static screens_info_t screens_info = {
    .height_A = 1440 + 1080 - 1920,
    .width_A = 1920 + 1080,
    .x_offset_A = 0,
    .y_offset_A = 0,
    .height_B = 1440,
    .width_B = 2048,
    .x_offset_B = 1920 - 2560,
    .y_offset_B = 1080 - 0
};

static inline int x_coord_A_to_abs(int x) { 
    return (x * screens_info.width_A / MAX_SCREEN_COORD) + screens_info.x_offset_A;
}

static inline int y_coord_A_to_abs(int y) { 
    return y * screens_info.height_A / MAX_SCREEN_COORD + screens_info.y_offset_A;
}

static inline int x_coord_abs_to_A(int x) { 
    return (x - screens_info.x_offset_A) * MAX_SCREEN_COORD / screens_info.width_A;
}

static inline int y_coord_abs_to_A(int y) { 
    return (y - screens_info.y_offset_A) * MAX_SCREEN_COORD / screens_info.height_A;
}

static inline int x_coord_B_to_abs(int x) { 
    return (x * screens_info.width_B / MAX_SCREEN_COORD) + screens_info.x_offset_B;
}

static inline int y_coord_B_to_abs(int y) { 
    return y * screens_info.height_B / MAX_SCREEN_COORD + screens_info.y_offset_B;
}

static inline int x_coord_abs_to_B(int x) { 
    return (x - screens_info.x_offset_B) * MAX_SCREEN_COORD / screens_info.width_B;
}

static inline int y_coord_abs_to_B(int y) { 
    return (y - screens_info.y_offset_B) * MAX_SCREEN_COORD / screens_info.height_B;
}

// Check if coord is on screen
static inline bool coord_on_screen(int x, int y) {
    return (x >= MIN_SCREEN_COORD && x <= MAX_SCREEN_COORD &&
            y >= MIN_SCREEN_COORD && y <= MAX_SCREEN_COORD);
}
