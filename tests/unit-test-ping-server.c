#include <stdio.h>
#include <stdlib.h>
#include <modbus.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "unit-test.h"

//#define SERVER_ID 1
#define REGISTER_ADDRESS 100
#define PING_VALUE 0x1234
#define PONG_VALUE 0x5678

long long current_millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
}

int main(int argc, char *argv[]) {
    modbus_t *ctx = NULL;
    modbus_mapping_t *mb_mapping;
    int rc;

    uint8_t response[RESPONSE_SIZE] = {0x01, 0x10, 0x00, 0x12, 0x34, 0x56, 0x78, 
                                       0x87, 0x65, 0x43, 0x21, 0x01, 0xAA, 0xBB, 0x12, 0x34};

    // Create Modbus RTU context
    //ctx = modbus_new_rtu("COM4", 2048000, 'N', 8, 1);
    ctx = modbus_new_rtu("COM4", 1024000, 'N', 8, 1);
    if (!ctx) {
        fprintf(stderr, "Failed to create modbus context\n");
        return -1;
    }
    
    // Set the slave ID
    modbus_set_slave(ctx, SERVER_ID);
    modbus_set_debug(ctx, TRUE);

    // Allocate registers
    mb_mapping = modbus_mapping_new(0, 0, 100, 0);
    if (!mb_mapping) {
        fprintf(stderr, "Failed to allocate mapping\n");
        modbus_free(ctx);
        return -1;
    }

    // Open serial port
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("Modbus RTU Server running on COM4...\n");

    while (1) {
        uint8_t query[REQUEST_SIZE];
        
        long long start_time = current_millis();

        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            long long end_time = current_millis();
            //modbus_reply(ctx, query, rc, mb_mapping);
            modbus_send_raw_request(ctx, response, RESPONSE_SIZE);
            
            // If "ping" received, store "pong"
            if (mb_mapping->tab_registers[REGISTER_ADDRESS] == PING_VALUE) {
                printf("Received PING from client.\n");
                mb_mapping->tab_registers[REGISTER_ADDRESS] = PONG_VALUE;
            }

            long long transfer_time = end_time - start_time;
            double bits_transfer = (REQUEST_SIZE + RESPONSE_SIZE) * 8;
            double speed_bps = bits_transfer / (transfer_time / 1000.0);
            printf("Frame received: %d bytes, Response sent: %d bytes\n", REQUEST_SIZE, RESPONSE_SIZE);
            printf("Time taken: %lld ms, Speed: %.2f bps (%.2f kbps)\n", transfer_time, speed_bps, speed_bps / 1000.0);
        } else if (rc == -1) {
            modbus_free(ctx);
            modbus_close(ctx);
            Sleep(1);
            if (modbus_connect(ctx) == -1) {
                printf("Unable to reconnect: %s\n", modbus_strerror(errno)); 
                break;
            }
        }
    }

    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
