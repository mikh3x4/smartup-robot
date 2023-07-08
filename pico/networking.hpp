
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

// #include "error.hpp"

//unused
enum ROBOT_STATE {RUNNING, ESTOP, LOW_BATTERY, INIT, MULIPLE_CONTROLLERS};

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

    void init_udp_receiver();

    void init();

    void send_udp();

};


void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

extern Networking main_data;
#define DEBUG_PRINT main_data.telemetry.debug_print

