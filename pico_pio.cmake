add_executable(cam_pio)

pico_generate_pio_header(cam_pio ${CMAKE_CURRENT_LIST_DIR}/cam.pio)

target_sources(cam_pio PRIVATE main.c board_i2c image_capture st7735 DEV_Config)

# pull in common dependencies
target_link_libraries(cam_pio PRIVATE pico_stdlib hardware_pio hardware_i2c hardware_pwm hardware_dma hardware_spi pico_multicore)

# create map/bin/hex file etc.
# pico_add_extra_outputs(cam_pio)