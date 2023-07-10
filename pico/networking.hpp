
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 8850
#define BEACON_MSG_LEN_MAX 1024
#define BEACON_TARGET "192.168.4.247"
#define BEACON_INTERVAL_MS 1000

#include "jsnm.h"

#include "command.hpp"
#include "telemetry.hpp"
#include "error.hpp"


enum ROBOT_STATE {RUNNING, ESTOP, LOW_BATTERY, INIT, MULIPLE_CONTROLLERS};

void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

class Networking {
public:
    enum ROBOT_STATE robot_data = INIT;
    struct pbuf *current_incomming; //and outgoing at the same time

    ParseJSON json_parser;

    // Double buffer the command so one is writen to while the other is used
    Command command_1;
    Command command_2;

    // then they are swapped
    // THIS IS VERY THREAD SAFTLY SKETCH
    // IT RELIES ON POINTER WRITES BEING ATTOMIC ON THE RP2040
    Command *active_command;
    Command *scratch_command;

    Telemetry telemetry;

    struct udp_pcb* udp_pcb;

    void init_udp_receiver() {
        struct udp_pcb* pcb = udp_new();
        ASSERT(pcb != NULL);

        err_t er = udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
        ASSERT(er == ERR_OK);

        udp_recv(pcb, udp_recv_callback, this);
    }

    void init(){
        ip_addr_t ip;
        ipaddr_aton("192.168.5.50", &ip);
        netif_set_ipaddr(netif_list, &ip);

        printf("IP address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
        
        this->udp_pcb = udp_new();
        this->active_command = &this->command_1;
        this->scratch_command = &this->command_2;

        ipaddr_aton(BEACON_TARGET, &(this->active_command->telemetry_address));
        this->active_command->telemetry_port = 8851;

        init_udp_receiver();
    }

    void send_udp() {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        char *buffer = (char *)p->payload;

        size_t real_size = this->telemetry.encode_json(buffer, BEACON_MSG_LEN_MAX);

        pbuf_realloc(p, real_size);

        err_t er = udp_sendto(this->udp_pcb, p, &(active_command->telemetry_address), active_command->telemetry_port);

        // printf("Sent packet: %.*s\n", p->len, buffer);

        pbuf_free(p);
        ASSERT(er == ERR_OK);
    }

};


void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p == NULL) return;

    Networking *main_data_ptr = (Networking*)arg;

    memcpy(&(main_data_ptr->scratch_command->telemetry_address), addr, sizeof(ip_addr_t));
    main_data_ptr->scratch_command->telemetry_port = port;
    main_data_ptr->scratch_command->recv_time = get_absolute_time();

    char *rec = (char *)p->payload;
    bool worked = main_data_ptr->json_parser.parse_message(rec, p->len, main_data_ptr->scratch_command);

    // printf("Received packet: %.*s\n", p->len ,rec);
    pbuf_free(p);

    if (worked){
        // printf("Parsed ok\n");

        Command *temp = main_data_ptr->active_command;
        main_data_ptr->active_command = main_data_ptr->scratch_command; // This line needs to be atomic
        main_data_ptr->scratch_command = temp;
    }
}

extern Networking main_data;
#define DEBUG_PRINT main_data.telemetry.debug_print

