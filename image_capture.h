#ifndef IMAGE_CAPTURE_H
#define IMAGE_CAPTURE_H

void main_dma(PIO pio, uint sm, uint dma_chan, uint8_t *capture_buf, size_t capture_size_words, 
                uint trigger_pin, bool trigger_level);

#endif