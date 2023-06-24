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

// testing cpp features
class Geeks {
public:
    char* geekname;
    void printname() { printf("testing"); }
};



char incoming[] = "{"
" \"s\": [ null, null, 91, null], "
" \"m\": [ null, [\"pwr\", 255], [\"spd\", 50], null], "
" \"led\": [ 255, 100, 0, 0], "
    "}";



class ParseJSON{

    jsmn_parser p;
    jsmntok_t t[128];
    jsmntok_t * current;

    void parse_message(){

    }

    void parse_motor(){
    }

    void parse_servo(){

        switch (current->type){
            JSMN_PRIMITIVE:
            default:
        }
    }

    void parse_led(){
        switch (current->type){
            JSMN_PRIMITIVE:
            default:
        }
    }
}


void core1_entry() {

    // multicore_fifo_push_blocking(FLAG_VALUE);
    // uint32_t g = multicore_fifo_pop_blocking();


    while (1){
        // tight_loop_contents();
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);
    }
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


    multicore_launch_core1(core1_entry);

    run_udp_receiver();
    run_udp_beacon();

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}










