#include <stdio.h>
#include <stdlib.h>
#include <modbus.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "unit-test.h"

#define REGISTER_ADDRESS 100
#define PING_VALUE 0x1234
#define PONG_VALUE 0x5678

long long current_millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
}

int main(int argc, char *argv[]) 
{
    modbus_t *ctx = NULL;
    uint16_t reg;
    int rc, i;

    long long start_time, end_time;
    uint8_t frame[REQUEST_SIZE] = {0x01, 0x02, 0x10, 0xAA, 0xBB, 0x00, 0x01, 
                                 0x00, 0x50, 0x00, 0x30, 0x00, 0x31, 0x00, 0x32, 0x00, 0x33, 
                                 0x00, 0x34, 0x00, 0x35, 0x00, 0x36, 0x00, 0x37, 0x12, 0x34, 
                                 0x56, 0x78, 0x87, 0x65, 0x43, 0x21};

    uint8_t response[RESPONSE_SIZE];

    // Create Modbus RTU context for COM4
    //ctx = modbus_new_rtu("COM5", 2048000, 'N', 8, 1);
    ctx = modbus_new_rtu("COM5", 1024000, 'N', 8, 1);
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create modbus context\n");
        return -1;
    }

    modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(
        ctx, MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);

    // Open serial port
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("Modbus RTU Client scanning devices on COM5...\n");

    // Iterate through all possible slave addresses (1 to 247)

    modbus_set_slave(ctx, SERVER_ID);

    start_time = current_millis();
    for (i = 1; i <= 1000; i++) {
        //modbus_set_slave(ctx, SERVER_ID);
        //modbus_set_slave(ctx, i);
        //printf("Trying address %d...\n", i);

        // Write "ping" (0x1234) to the server
        //rc = modbus_write_register(ctx, REGISTER_ADDRESS, PING_VALUE);
        rc = modbus_send_raw_request(ctx, frame, REQUEST_SIZE);
        if (rc == -1) {
            printf("No response.\n");
            continue;
        }

        // Wait a moment for the server to process
        //modbus_sleep(500);

        // Read back the value
        //rc = modbus_read_registers(ctx, REGISTER_ADDRESS, 1, &reg);
        rc = modbus_receive_confirmation(ctx, response);
        if (rc == -1) {
            printf("Read failed.\n");
            continue;
        }

        // Check if response is "pong" (0x5678)
        //if (reg == PONG_VALUE) {
        //    printfSuccess! PONG received from device %d\n", i);
        //} else {
        //    printf("Unexpected response: 0x%X\n", reg);
        //}
    }

    end_time = current_millis();

    // Calculate actual speed
    long long total_time = end_time - start_time; // in ms
    double seconds = total_time / 1000.0;
    double bits_transferred = 1000.0 * (REQUEST_SIZE + RESPONSE_SIZE) * 8;
    double speed_bps = bits_transferred / seconds;

    printf("Total time: %lld ms\n", total_time);
    printf("Estimated data speed: %.2f bps (%.2f kbps)\n", speed_bps, speed_bps / 1000.0);

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
