#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include "hardware/vreg.h"
#include "pico/float.h"
#include "build/cam.pio.h"
#include "board_i2c.h"
#include "image_capture.h"
#include "st7735.h"
uint8_t capture_buffer[244][324];
uint8_t displayBuf[80*160*2];

#define FLAG_VALUE 123

void core1_entry() {
    /*
    multicore_fifo_push_blocking(FLAG_VALUE);

    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE) {
      printf("Hmm, that's not right on core 1!\n");
    } else {
      printf("It's all gone well on core 1!\n");
    }
    */
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &cam_program);
    uint sm = pio_claim_unused_sm(pio, true);
    uint dma_chan = 0;
    uint dma_chan1 = dma_claim_unused_channel(true);

    cam_program_init(pio, sm, offset, 6);

    cam_init();
    cam_config(pio, sm);
    ST7735_Init();
    

    while (true) {
        main_dma(pio, sm, dma_chan, capture_buffer, sizeof(capture_buffer), 16, 1);
	    uint16_t index = 0;
	    for (int y = 0; y < 160; y++) {
	        for (int x = 0; x < 80; x++) {
                uint8_t ny = (uint8_t)(2+240-(1.5125)*y);
                uint8_t nx = (uint8_t)(2+40+1.5125*x);
                uint8_t c = capture_buffer[ny][nx];
                //printf("%i = %i\n", (2+320-2*y)*244+(2+40+2*x), capture_buffer[(2+320-2*y)*244+(2+40+2*x)]);
                uint16_t imageRGB   = ST7735_COLOR565(c, c, c);
                displayBuf[index++] = (uint8_t)(imageRGB >> 8);// & 0xFF;
                displayBuf[index++] = (uint8_t)(imageRGB);//&0xFF;
            }
	    }
	    ST7735_DrawImage(0, 0, 80, 160, displayBuf);
	}
}

int main() {
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    sleep_ms(1000);
    set_sys_clock_khz(250000, true);
    stdio_init_all();
    gpio_set_dir(16, false);

    multicore_launch_core1(core1_entry);
    /*
    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE) {
        printf("Hmm, that's not right on core 0!\n");
    } else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!\n");
    }*/
    while (1) {
        tight_loop_contents();
    }
}