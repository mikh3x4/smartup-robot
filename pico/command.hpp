
#pragma once

#include<stdio.h>
#include<stdint.h>

// #include "command.hpp"
#include "jsnm.h"
#include "error.hpp"

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint16_t blink;
} Led;


typedef struct {
    bool on = false;
    int pulse_width = 1500;
} Servo;


enum MOTOR_MODE {OFF, POWER, SPEED, DISTANCE};

typedef struct {
    enum MOTOR_MODE mode = OFF;
    int16_t power;
    int16_t speed;
    int32_t distance;

    float ff = 1.0;
    float kp = 1.0;
    float ki = 0.0;
    float kd = 0.0;
} Motor;


typedef struct {
    Led led;
    Motor motors[4];
    Servo servos[4];

    ip_addr_t telemetry_address;
    u16_t telemetry_port;
    absolute_time_t recv_time;

    void estop(){
        led.red = 255;
        led.green = 0;
        led.blue = 0;
        led.blink = 100;

        motors[0].mode = OFF;
        motors[1].mode = OFF;
        motors[2].mode = OFF;
        motors[3].mode = OFF;

        servos[0].on = false;
        servos[1].on = false;
        servos[2].on = false;
        servos[3].on = false;
    }

} Command;


#define MAX_JSON_TOKENS 128

class ParseJSON{

    jsmn_parser parser;
    jsmntok_t tokens[MAX_JSON_TOKENS];
    jsmntok_t * current;

    char * js;
    Command * command_struct;

public:
    bool parse_message(char *incoming_str, size_t incoming_len, Command * where_to_save){
        command_struct = where_to_save;
        js = incoming_str;

        jsmn_init(&parser);
        int token_number = jsmn_parse(&parser, js, incoming_len, tokens, MAX_JSON_TOKENS);

        if (token_number < 0){
            switch (token_number){
                case JSMN_ERROR_INVAL: printf("invalid json\n"); break;
                case JSMN_ERROR_NOMEM: printf("insufficent tokens\n"); break;
                case JSMN_ERROR_PART: printf("partial json\n"); break;
                default: printf("unknown error\n");
            }
            return false;
        }

        current = tokens;

        // while ( current < tokens + token_number) {
        //     printf(" type: %d, start: %d, end: %d, size: %d\n", current->type, current->start, current->end, current->size);
        //     current++;
        // }
        // current = tokens;

        ASSERT_RETURN(current->type == JSMN_OBJECT);
        current++;

        ASSERT_RETURN(current->type == JSMN_STRING);

        while ( current < tokens + token_number) {
            switch( js[current->start] ){
                case 's': current++; ASSERT_RETURN(parse_servos()); break;
                case 'm': current++; ASSERT_RETURN(parse_motors()); break;
                case 'l': current++; ASSERT_RETURN(parse_led()); break;
                case 'P': current++; ASSERT_RETURN(parse_pid()); break;
                case 'p': current++; ASSERT_RETURN(parse_password()); break;
                default: return false;
            }
        }
        return true;

    }

    bool parse_password(){
        ASSERT_RETURN(current->type == JSMN_STRING);
        // we put the quote in the string to verify we reached the end of the pass
        ASSERT_RETURN(0 == strncmp("secret\"", &js[current->start], current->end - current->start + 1));
        current++;
        return true;
    }

    bool parse_pid(){
        ASSERT_RETURN(current->type == JSMN_ARRAY);
        ASSERT_RETURN(current->size == 4);
        current++;

        ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
        int i = parse_int();
        current++;

        ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
        command_struct->motors[i].kp = parse_int();
        current++;

        ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
        command_struct->motors[i].ki = parse_int();
        current++;

        ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
        command_struct->motors[i].kd = parse_int();
        current++;

        return true;
    }


    bool parse_motor(int i){
        ASSERT_RETURN(current->type == JSMN_ARRAY);
        int size = current->size;
        current++;

        ASSERT_RETURN(current->type == JSMN_STRING);
        switch( js[current->start] ){
            case 'o': 
                ASSERT_RETURN(size == 1);
                command_struct->motors[i].mode = OFF;
                current++;
                break;
            case 'p': 
                ASSERT_RETURN(size == 2);
                command_struct->motors[i].mode = POWER;
                current++;
                ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
                command_struct->motors[i].power = parse_int();
                current++;
                break;

            case 's':
                ASSERT_RETURN(size == 2);
                command_struct->motors[i].mode = SPEED;
                current++;
                ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
                command_struct->motors[i].speed = parse_int();
                current++;
                break;
            case 'd':
                ASSERT_RETURN(size == 3);
                command_struct->motors[i].mode = DISTANCE;
                current++;
                ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
                command_struct->motors[i].power = parse_int();
                current++;

                ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
                command_struct->motors[i].distance = parse_int();
                current++;
                break;

            default: return false;
        }
        return true;

    }

    bool parse_motors(){
        ASSERT_RETURN(current->type == JSMN_ARRAY);
        ASSERT_RETURN(current->size == 4);
        current++;

        for(int i = 0; i < 4; i++){
            ASSERT_RETURN(parse_motor(i));
        }
        return true;
    }

    bool parse_servos(){
        ASSERT_RETURN(current->type == JSMN_ARRAY);
        ASSERT_RETURN(current->size == 4);
        current++;

        for(int i = 0; i < 4; i++){
            ASSERT_RETURN(parse_servo(i));
        }
        return true;
    }

    bool parse_servo(int i){
        ASSERT_RETURN(current->type == JSMN_PRIMITIVE);
        switch( js[current->start] ){
            case 'n': command_struct->servos[i].on = false; break;
            default: 
                command_struct->servos[i].on = true;
                command_struct->servos[i].pulse_width = parse_int();
        }

        current++;
        return true;
    }

    long parse_int(){
        ASSERT(current->type == JSMN_PRIMITIVE);

        char first_char = js[current->start];
        ASSERT(first_char == '-' or (first_char >= '0' and first_char <= '9'));

        return strtol(&js[current->start], NULL, 10);
    }

    bool parse_led(){
        ASSERT_RETURN(current->type == JSMN_ARRAY);
        ASSERT_RETURN(current->size == 4);
        current++;

        command_struct->led.red = parse_int();
        current++;

        command_struct->led.green = parse_int();
        current++;

        command_struct->led.blue = parse_int();
        current++;

        command_struct->led.blink = parse_int();
        current++;
        return true;
    }
};


