#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.hpp"

#define DEBUG_STRING_LEN 128

class Telemetry {
    public:
        long encoders_position[4] = {0, 1, 10, 100};
        long encoders_speed[4] = {0, 5, 50, -500};
        float v_bat = 3.14;
        float temp = 20.0; //probably not needed but added as test
        bool motor_done[4] = {0, 0, 0, 1};
        long settings_version = 0;
        char debug[DEBUG_STRING_LEN] = "testing";

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

