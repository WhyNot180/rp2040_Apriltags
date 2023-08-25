#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "board_i2c.h"


void cam_init(void) {
    i2c_init(i2c_default, 200000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    gpio_set_function(3, GPIO_FUNC_PWM);
    gpio_set_dir(3, false);
    pwm_set_clkdiv_int_frac(1, 1, 0); //1, 1, 4
    pwm_set_wrap(1, 4); //1, 2
    pwm_set_chan_level(1, 1, 2); //1, 1, 1
    pwm_set_enabled(1, true);
}

void cam_write(const uint8_t *data, size_t length) {
    i2c_write_blocking(i2c_default, 0x24, data, length, false);
}

void cam_config(PIO pio, uint sm) {
    uint8_t reset[3] = {0x01, 0x03, 0x00}; //Resets camera
    uint8_t standby_mode[3] = {0x01, 0x00, 0x00}; //Enables standby mode

    uint8_t clk[3] = {0x30, 0x67, 0x01}; //switches to MCLK mode
    uint8_t oscDiv[3] = {0x30, 0x60, 0x38}; //Enables MSB and divides sensor core by 8

    uint8_t test_pattern[3] = {0x06, 0x01, 0x00}; //Enables test pattern and sets to walking 1s

    //uint8_t aGain[3] = {0x02, 0x05, 0x70}; //Sets Analog gain to 8x (aGain x dGain = final gain)
    //uint8_t dGain[3] = {0x02, 0x0E, 0x02}; //Sets Digital gain to 2x (aGain x dGain = final gain)

    uint8_t stream_mode[3] = {0x01, 0x00, 0x01}; //Enables full stream mode

    uint8_t bitControl[3] = {0x30, 0x59, 0x22}; //sets to 1 bit transfer (default is 0x02)
    uint8_t format[3] = {0x30, 0x11, 0x70}; //Enables 6-bit mode

    uint8_t dpc[3] = {0x10, 0x08, 0x00}; //Enables Mono on dpc

    uint8_t blt[3] = {0x10, 0x03, 0x08}; //sets black level target to 8
    uint8_t blc2[3] = {0x10, 0x07, 0x08}; //sets BLC2 to same as BLT
    uint8_t bl_en[3] = {0x10, 0x00, 0x01}; //Enables black level correction
    uint8_t bli_en[3] = {0x10, 0x06, 0x01}; //Enables BLI

    uint8_t vsync_adv[3] = {0x30, 0x22, 0x01}; //Advances vysnc by one row at a time
    uint8_t shift_en[3] = {0x10, 0x12, 0x01}; //Enables vsync shift

    uint8_t ae_en[3] = {0x21, 0x00, 0x01}; //Enables auto exposure
    uint8_t ae_target[3] = {0x21, 0x01, 0x5F}; //Sets auto exposure target mean 0x5F
    uint8_t ae_min[3] = {0x21, 0x02, 0x0A}; //Sets auto exposure min mean
    uint8_t ae_conv_in[3] = {0x21, 0x03, 0x05}; //Sets auto exposure converge in threshold
    uint8_t ae_conv_out[3] = {0x21, 0x04, 0x05}; //Sets auto exposure converge out threshold
    uint8_t ae_int_max_H[3] = {0x21, 0x05, 0x02}; //Sets auto exposure MSB of max integration
    uint8_t ae_int_max_L[3] = {0x21, 0x06, 0x14}; //Sets auto exposure LSB of max integration
    uint8_t ae_int_min[3] = {0x21, 0x07, 0x02}; //Sets auto exposure min integration
    uint8_t ae_aGain_max_full[3] = {0x21, 0x08, 0x70}; //Sets auto exposure max aGain in full frame mode
    uint8_t ae_aGain_max_bin[3] = {0x21, 0x09, 0x70}; //Sets auto exposure max aGain in bin2 mode
    uint8_t ae_aGain_min[3] = {0x21, 0x0A, 0x00}; //Sets auto exposure min aGain
    uint8_t ae_dGain_max[3] = {0x21, 0x0B, 0xC0}; //Sets auto exposure max dGain
    uint8_t ae_dGain_min[3] = {0x21, 0x0C, 0x40}; //Sets auto exposure min dGain
    uint8_t ae_damp[3] = {0x21, 0x0D, 0x20}; //Sets auto exposure damping factor
    uint8_t ae_fs_ctrl[3] = {0x21, 0x0E, 0x03}; //Sets auto exposure flicker step control
    uint8_t ae_fs_60hz_H[3] = {0x21, 0x0F, 0x00}; //Sets auto exposure MSB of 60hz flicker ctrl
    uint8_t ae_fs_60hz_L[3] = {0x21, 0x10, 0x85}; //Sets auto exposure LSB of 60hz flicker ctrl
    uint8_t ae_fs_50hz_H[3] = {0x21, 0x11, 0x00}; //Sets auto exposure MSB of 50hz flicker ctrl
    uint8_t ae_fs_50hz_L[3] = {0x21, 0x12, 0xA0}; //Sets auto exposure LSB of 50hz flicker ctrl

    uint8_t motion_en[3] = {0x21, 0x50, 0x00}; //Disables motion detection

    uint8_t frame_lenH[3] = {0x03, 0x40, 0x00}; //Sets frame length MSB to 0x01
    uint8_t frame_lenL[3] = {0x03, 0x41, 0x80}; //Sets frame length LSB to 0x7A
    uint8_t line_lenH[3] = {0x03, 0x42, 0x00}; //Sets line length MSB to 0x01
    uint8_t line_lenL[3] = {0x03, 0x43, 0xD7}; //Sets line length LSB to 0x77

    //uint8_t integH[3] = {0x02, 0x02, 0x01}; //Sets integration time MSB to 0x01
    //uint8_t integL[3] = {0x02, 0x03, 0x78}; //Sets integration time LSB to 0x78

    uint8_t bin_x[3] = {0x03, 0x83, 0x03}; //Disables binning for x
    uint8_t bin_y[3] = {0x03, 0x87, 0x03}; //Disables binning for y
    uint8_t bin_en[3] = {0x03, 0x90, 0x01}; //Enables binning

    uint8_t flip[3] = {0x01, 0x01, 0x01}; //Flips image horizontally and vertically

    uint8_t grp[3] = {0x01, 0x04, 0x01}; //Sets group parameter to hold

    uint8_t qvga[3] = {0x30, 0x10, 0x01}; //Enables QVGA (324 x 244)

    i2c_write_blocking(i2c_default, 0x24, reset, 3, false);
    i2c_write_blocking(i2c_default, 0x24, standby_mode, 3, false);

    i2c_write_blocking(i2c_default, 0x24, clk, 3, false);
    i2c_write_blocking(i2c_default, 0x24, oscDiv, 3, false);

    i2c_write_blocking(i2c_default, 0x24, bitControl, 3, false);
    i2c_write_blocking(i2c_default, 0x24, format, 3, false);

    //i2c_write_blocking(i2c_default, 0x24, aGain, 3, false);
    //i2c_write_blocking(i2c_default, 0x24, dGain, 3, false);

    i2c_write_blocking(i2c_default, 0x24, dpc, 3, false);

    i2c_write_blocking(i2c_default, 0x24, blt, 3, false);
    i2c_write_blocking(i2c_default, 0x24, blc2, 3, false);
    i2c_write_blocking(i2c_default, 0x24, bl_en, 3, false);
    i2c_write_blocking(i2c_default, 0x24, bli_en, 3, false);

    i2c_write_blocking(i2c_default, 0x24, vsync_adv, 3, false);
    i2c_write_blocking(i2c_default, 0x24, shift_en, 3, false);
    
    i2c_write_blocking(i2c_default, 0x24, ae_en, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_target, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_min, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_conv_in, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_conv_out, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_int_max_H, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_int_max_L, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_int_min, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_aGain_max_full, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_aGain_max_bin, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_aGain_min, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_dGain_max, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_dGain_min, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_damp, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_fs_ctrl, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_fs_60hz_H, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_fs_60hz_L, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_fs_50hz_H, 3, false);
    i2c_write_blocking(i2c_default, 0x24, ae_fs_50hz_L, 3, false);

    i2c_write_blocking(i2c_default, 0x24, motion_en, 3, false);

    i2c_write_blocking(i2c_default, 0x24, frame_lenH, 3, false);
    i2c_write_blocking(i2c_default, 0x24, frame_lenL, 3, false);
    i2c_write_blocking(i2c_default, 0x24, line_lenH, 3, false);
    i2c_write_blocking(i2c_default, 0x24, line_lenL, 3, false);

    //i2c_write_blocking(i2c_default, 0x24, integH, 3, false);
    //i2c_write_blocking(i2c_default, 0x24, integL, 3, false);

    i2c_write_blocking(i2c_default, 0x24, bin_x, 3, false);
    i2c_write_blocking(i2c_default, 0x24, bin_y, 3, false);
    i2c_write_blocking(i2c_default, 0x24, bin_en, 3, false);

    i2c_write_blocking(i2c_default, 0x24, flip, 3, false);

    i2c_write_blocking(i2c_default, 0x24, grp, 3, false);

    i2c_write_blocking(i2c_default, 0x24, qvga, 3, false);

    i2c_write_blocking(i2c_default, 0x24, test_pattern, 3, false);

    i2c_write_blocking(i2c_default, 0x24, stream_mode, 3, false);
    // cam_write(reset, 3);
    // cam_write(standby_mode, 3);
    // cam_write(clk, 3);
    // cam_write(oscDiv, 3);
    // cam_write(bitControl, 3);
    // cam_write(aGain, 3);
    // cam_write(dGain, 3);
    // cam_write(format, 3);
    // cam_write(dpc, 3);
    // cam_write(blt, 3);
    // cam_write(blc2, 3);
    // cam_write(bl_en, 3);
    // cam_write(bli_en, 3);
    // cam_write(vsync_adv, 3);
    // cam_write(shift_en, 3);
    // cam_write(ae_en, 3);
    // cam_write(motion_en, 3);
    // cam_write(frame_lenH, 3);
    // cam_write(frame_lenL, 3);
    // cam_write(line_lenH, 3);
    // cam_write(line_lenL, 3);
    // cam_write(bin_x, 3);
    // cam_write(bin_y, 3);
    // cam_write(bin_en, 3);
    // cam_write(flip, 3);
    // cam_write(grp, 3);
    // cam_write(qvga, 3);
    // cam_write(stream_mode, 3);
}