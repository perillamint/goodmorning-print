#include "driver/uart.h"
#include "dkb844.h"
#include "latin.h"
#include "utf8dec.h"
#include "string.h"

#define BUF_SIZE (1024)

const int uart_num = UART_NUM_2;
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
    .rx_flow_ctrl_thresh = 122,
};

void printer_init() {
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, \
                                        uart_buffer_size, 10, &uart_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, 17, 16, 5, 18));
    dkb_init();
    latinfnt_init();
}

void println(char* msg) {
    uint8_t buf[1024];
    memset(buf, 0, 1024);
    uint8_t dkbbuf[32];

    int _width = 512;
    int _height = 16;

    uart_write_bytes(uart_num, "\x1B@", 2);
    snprintf((char*)buf, 32, "\x1D\x76\x30%c%c%c%c%c", 0, (_width / 8) % 256, 0, _height % 256, _height / 256);
    uart_write_bytes(uart_num, (char*)buf, 8);

    char* msgptr = msg;
    int off = 0;
    for(int j = 0; off < 64; j++) {
        uint32_t tmp = 0;
        int len = utf8_getcodept((uint8_t*) msgptr, strlen(msgptr), &tmp);
        if (len < 0) {
            break;
        }
        msgptr += len;

        int charwidth = 1;
        if (tmp < 0xFF) {
            latinfnt_render(tmp, dkbbuf);
        } else {
            dkb_render(tmp, dkbbuf);
            charwidth = 2;
        }

        for(int k = 0; k < 16; k++) {
            buf[off + (_width / 8) * k] = dkbbuf[k * charwidth];
            if (charwidth > 1) {
                buf[off + 1 + (_width / 8) * k] = dkbbuf[k * charwidth + 1];
            }
        }
        off += charwidth;
    }

    // Print fb
    uart_write_bytes(uart_num, (char*)buf, 1024);
}

void print_feed(float dist) {
    uint8_t buf[32];
    snprintf((char*)buf, 32, "\x1BJ%c", (int)(dist / 0.0705 / 2)); // Line feed
    uart_write_bytes(uart_num, (char*)buf, 3);
}

void print_cut() {
    print_feed(19);
    uart_write_bytes(uart_num, "\x1Bi", 2); // Cut
}
