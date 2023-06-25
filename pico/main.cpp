/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// #include "pico/stdlib.h"
//
// int main() {
// #ifndef PICO_DEFAULT_LED_PIN
// #warning blink example requires a board with a regular LED
// #else
//     const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
//     while (true) {
//         gpio_put(LED_PIN, 1);
//         sleep_ms(250);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(250);
//     }
// #endif
// }


/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"

#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 8850
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "192.168.4.247"
#define BEACON_INTERVAL_MS 1000

#include "pico/multicore.h"

#include "jsnm.h"
#include "command.hpp"

#include <string.h>

#include "wifi_pass.h"


char incoming[] = "{"
" \"s\": [ null, null, 91, null], "
" \"led\": [ 255, 100, 0, 0], "
    "}";

// " \"m\": [ null, [\"pwr\", 255], [\"spd\", 50], null], "

void panic(){
    while (1){
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
    }
}

void assert_true(bool should_be_true){
    if (not should_be_true){
        panic();
    }
}

#define MAX_JSON_TOKENS 128

class ParseJSON{

    jsmn_parser parser;
    jsmntok_t tokens[MAX_JSON_TOKENS];
    jsmntok_t * current;

    char * js;
    Command * command_struct;

public:
    void parse_message(char *incoming_str, Command * where_to_save){
        command_struct = where_to_save;
        js = incoming_str;

        jsmn_init(&parser);
        int token_number = jsmn_parse(&parser, js, strlen(js), tokens, MAX_JSON_TOKENS);

        if (token_number < 0){
            switch (token_number){
                case JSMN_ERROR_INVAL: printf("invalid json\n"); break;
                case JSMN_ERROR_NOMEM: printf("insufficent tokens\n"); break;
                case JSMN_ERROR_PART: printf("partial json\n"); break;
                default: printf("unknown error\n");
            }
            assert_true(false);
        }

        current = tokens;

        while ( current < tokens + token_number) {
            printf(" type: %d, start: %d, end: %d, size: %d\n", current->type, current->start, current->end, current->size);
            current++;
        }
        current = tokens;

        assert_true(current->type == JSMN_OBJECT);
        current++;

        assert_true(current->type == JSMN_STRING);

        while ( current < tokens + token_number) {
            switch( js[current->start] ){
                case 's': current++; parse_servos(); break;
                case 'm': current++; parse_motors(); break;
                case 'l': current++; parse_led(); break;
                default: assert_true(false);
            }
        }

    }

    void parse_motor(int i){
    }

    void parse_motors(){
        assert_true(current->type == JSMN_ARRAY);
        current++;

        for(int i = 0; i < 4; i++){
            parse_motor(i);
        }
    }

    void parse_servos(){
        assert_true(current->type == JSMN_ARRAY);
        current++;

        for(int i = 0; i < 4; i++){
            parse_servo(i);
        }
    }

    void parse_servo(int i){
        assert_true(current->type == JSMN_PRIMITIVE);
        switch( js[current->start] ){
            case 'n': command_struct->servos[i].on = false; break;
            default: 
                command_struct->servos[i].on = true;
                command_struct->servos[i].angle = parse_int();
        }

        current++;
    }

    long parse_int(){
        assert_true(current->type == JSMN_PRIMITIVE);

        char first_char = js[current->start];
        assert_true(first_char == '-' or (first_char >= '0' and first_char <= '9'));

        return strtol(&js[current->start], NULL, 10);
    }

    void parse_led(){
        assert_true(current->type == JSMN_ARRAY);
        current++;

        command_struct->led.red = parse_int();
        current++;

        command_struct->led.green = parse_int();
        current++;

        command_struct->led.blue = parse_int();
        current++;

        command_struct->led.blink = parse_int();
        current++;
    }
};


void core1_entry() {
        printf("hello from core 1\n");
}


void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    /* We have received a packet, process it. */
    if (p != NULL) {
        char *rec = (char *)p->payload;
        printf("Received packet: %s\n", rec);

        /* Free the pbuf */
        pbuf_free(p);
    }
}

void run_udp_receiver() {
    struct udp_pcb* pcb = udp_new();
    assert_true(pcb != NULL);

    if (pcb == NULL) {
        printf("Failed to create new UDP PCB.\n");
        return;
    }

    err_t er = udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
    assert_true(er == ERR_OK);

    udp_recv(pcb, udp_recv_callback, NULL);

}


void run_udp_beacon() {
    struct udp_pcb* pcb = udp_new();

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 0;
    while (true) {


        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        char *req = (char *)p->payload;
        memset(req, 0, BEACON_MSG_LEN_MAX+1);
        snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p);
        if (er != ERR_OK) {
            printf("Failed to send UDP packet! error=%d", er);
        } else {
            printf("Sent packet %d\n", counter);
            counter++;
        }

        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(BEACON_INTERVAL_MS);
    }
}

void send_udp() {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        char *buffer = (char *)p->payload;

        main_data.sensors.encode_json(buffer, BEACON_MSG_LEN_MAX);

        // memset(req, 0, BEACON_MSG_LEN_MAX+1);
        // snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);

        // this will send a longer then needed message
        err_t er = udp_sendto(main_data.udp_pcb, p, &main_data.telemetry_address, main_data.telemetry_port);

        pbuf_free(p);

        assert_true(er == ERR_OK);
}


class Sensors {
    public:
        long encoders_position[4];
        long encoders_speed[4];
        float v_bat;
        bool motor_done[4];
        long settings_version;

    size_t encode_json(char *js, size_t len){
        int writen = snprintf(js, len,
                 "{ \"vbat\": %.2f, "
                 "\"pos\": [%ld, %ld, %ld, %ld], "
                 "\"spd\": [%ld, %ld, %ld, %ld], "
                 "\"done\": [%d, %d, %d, %d], "
                 "\"set_ver\": %ld }",
                 v_bat,
                 encoders_position[0], encoders_position[1], encoders_position[2], encoders_position[3],
                 encoders_speed[0], encoders_speed[1], encoders_speed[2], encoders_speed[3],
                 motor_done[0], motor_done[1], motor_done[2], motor_done[3],
                 settings_version);

        assert( writen > 0);
        assert( writen < len);
        return writen;
    }
};


enum ROBOT_STATE {RUNNING, ESTOP, LOW_BATTERY, INIT, MULIPLE_CONTROLLERS};

class MainData {
public:
    Command command_1;
    Command command_2;

    Command *active_command;
    Command *scratch_command;

    Sensors sensors;

    struct udp_pcb* udp_pcb;
    ip_addr_t telemetry_address;
    u16_t telemetry_port;
    // last update time variable
};

MainData main_data;


// void loop(){
//
    // main_data.udp_pcb = udp_new();
//     get_udp;
//     if updated_udp{
//         parse_message
//         if successful { Command temp = scratch_command;
//             scratch_command = active_command;
//             active_command = temp;
//         }
//     }
//
//     if estop (
//         zero_everything;
//     )
//
//     run_actuators();
//
//     send_udp;
// }

void init(){
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf(WIFI_SSID);
    printf(" Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }
}

int main() {
    init();

    Command command;

    ParseJSON json_parser;

    printf("%d\n", command.servos[2].angle);

    json_parser.parse_message(incoming, &command);

    printf("%d\n", command.servos[2].angle);

    // multicore_launch_core1(core1_entry);

    // run_udp_receiver();
    // run_udp_beacon();

    printf("Hello, world!\n");

}










