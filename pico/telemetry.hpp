#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>

#include "error.hpp"

#define DEBUG_STRING_LEN 256

class Telemetry {
    public:
        long encoders_position[4] = {0, 1, 10, 100};
        long encoders_speed[4] = {0, 5, 50, -500};
        float v_bat = 3.14;
        float temp = 20.0; //probably not needed but added as test
        bool motor_done[4] = {0, 0, 0, 1};
        long settings_version = 0;
        char debug[DEBUG_STRING_LEN] = "testing";

    void clear_debug(){
        debug[0] = '\0';
    }

    // int snprintf(char *str, size_t size, const char *format, ...);
    size_t debug_print(const char * format, ...){
        size_t bufferLength = strlen(debug);

        ASSERT(bufferLength < DEBUG_STRING_LEN);
        if (bufferLength + 1 == DEBUG_STRING_LEN)
            return 0;

        va_list args;
        va_start(args, format);
        size_t written = vsnprintf(debug + bufferLength , DEBUG_STRING_LEN - bufferLength, format, args);
        va_end(args);

        return written;
    }


    size_t encode_json(char *js, size_t len){
        int writen = snprintf(js, len,
                 "{ \"vbat\": %.2f, "
                 "\"pos\": [%ld, %ld, %ld, %ld], "
                 "\"spd\": [%ld, %ld, %ld, %ld], "
                 "\"done\": [%d, %d, %d, %d], "
                 "\"debug\": \"%s\", "
                 "\"temp\": %.2f, "
                 "\"set_ver\": %ld }",
                 v_bat,
                 encoders_position[0], encoders_position[1], encoders_position[2], encoders_position[3],
                 encoders_speed[0], encoders_speed[1], encoders_speed[2], encoders_speed[3],
                 motor_done[0], motor_done[1], motor_done[2], motor_done[3],
                 debug,
                 temp,
                 settings_version);

        ASSERT(writen > 0);
        ASSERT(writen < len);
        return writen;
    }
};

