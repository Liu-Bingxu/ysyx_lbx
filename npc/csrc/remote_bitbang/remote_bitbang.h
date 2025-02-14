#ifndef REMOTE_BITBANG_H
#define REMOTE_BITBANG_H

#include <stdint.h>
#include "utils.h"
#include "regs.h"

// #include "jtag_dtm.h"
#include VTOP_H

#define RUN_TEST_IDLE 1

extern void step_and_dump_wave();
extern void sim_exit();

class remote_bitbang_t
{
public:
    // Create a new server, listening for connections from localhost on the given
    // port.
    remote_bitbang_t(uint16_t port, VTOP *tap);

    // Do a bit of work.
    void tick();

private:
    VTOP *tap;

    int socket_fd;
    int client_fd;

    static const ssize_t buf_size = 64 * 1024;
    char send_buf[buf_size];
    char recv_buf[buf_size];
    ssize_t recv_start, recv_end;

    // Check for a client connecting, and accept if there is one.
    void accept();
    // Execute any commands the client has for us.
    void execute_commands();

    // set tap pins
    void set_pins(uint8_t tck, uint8_t tms, uint8_t tdi);
    // reset tap
    void reset();
};

#endif
