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
    if (pcb == NULL) {
        printf("Failed to create new UDP PCB.\n");
        return;
    }

    err_t er = udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
    if (er != ERR_OK) {
        printf("Failed to bind to the port! error=%d", er);
        return;
    }

    /* set recv callback for any new udp packets that arrive at the port */
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



class Sensors {
    public:
        long encoders[4];
        float v_bat;
        bool motor_done[4];
        long settings_version;

    void encoder_json(char *js, size_t len){

        snprintf(js, len, " { \" vbat \" : \"%f\"  } ", v_bat );

    }
};

class MainData {
public:
    Command command_1;
    Command command_2;

    Command *active_command;
    Command *scratch_command;

    Sensors sensors;
};


int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("before ");
    printf(WIFI_SSID);
    printf(" after\n");

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }

    Command command;

    ParseJSON json_parser;

    json_parser.parse_message(incoming, &command);

    printf("%d\n", command.servos[2].angle);

    // multicore_launch_core1(core1_entry);

    // run_udp_receiver();
    // run_udp_beacon();

    printf("Hello, world!\n");

}










