#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "demos/lv_demos.h"

#include "LVGL_Driver.h"
// #include "SD_SPI.h"
// #include "Wireless.h"

#define EXAMPLE1_LVGL_TICK_PERIOD_MS  1000

// void lv_example_ui(void);
void set_brightness(int brightness);
void set_lcd_battery(unsigned int v);
void set_lcd_counter(uint16_t* v);
void set_main_screen();
void set_wifi_setup_screen();
// void set_sending_datalog_screen();
void set_datalog_sent_screen(const char* tx_hash);
bool is_current_screen_main();
void set_sending_datalog_screen(const char *data);
void set_connecting_screen(const char *ssid);
void set_not_connected_screen();
void set_connected_screen();
void set_datalog_not_sent_screen(const char *error_msg);

#ifdef __cplusplus
}
#endif