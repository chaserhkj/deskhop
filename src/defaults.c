#include "main.h"

/* Default configuration */
const config_t default_config = {
    .magic_header = 0xB00B1E5,
    .version = CURRENT_CONFIG_VERSION,
    .output[OUTPUT_A] =
        {
            .os = OUTPUT_A_OS,
        },        
    .output[OUTPUT_B] =
        {
            .os = OUTPUT_B_OS,
        },
};