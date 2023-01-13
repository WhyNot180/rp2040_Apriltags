#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "build/cam.pio.h"
#include "image_capture.h"
#include "hardware/dma.h"
#include <stdio.h>
#include "pico/stdio.h"

void main_dma(PIO pio, uint sm, uint dma_chan, uint8_t *capture_buf, size_t capture_size_words, 
                uint trigger_pin, bool trigger_level) 
{
    pio_sm_set_enabled(pio, sm, false);

    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);

    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);

    dma_channel_configure(dma_chan, &c,
        capture_buf,
        &pio->rxf[sm],
        capture_size_words,
        false
    );
    
    // dma_channel_configure(dma_chan1, &d,
    //     capture_buf,
    //     &pio->rxf[sm],
    //     capture_size_words,
    //     false
    // );
    // Wait for vsync rising edge to start frame
	while (gpio_get(16) == true);
	while (gpio_get(16) == false);
	
	dma_channel_start(dma_chan);
	pio_sm_set_enabled(pio, sm, true);
	dma_channel_wait_for_finish_blocking(dma_chan);
	pio_sm_set_enabled(pio, sm, false);
    //pio_sm_exec(pio, sm, pio_encode_wait_gpio(trigger_level, trigger_pin));
    //pio_sm_set_enabled(pio, sm, true);
}

