/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 *
 * Zigbee HA_on_off_light Example
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include "main.h"
#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "freertos/task.h"
// #include "ha/esp_zigbee_ha_standard.h"
#include "web_server.h"
#include "esp_coexist.h"
#include "utils/nvs_utils.h"
#include "LCD_Driver/ST7789.h"
#include "RGB/RGB.h"
#include "LVGL_UI/LVGL_Example.h"
#include "driver/gpio.h"
#include "utils/battery_utils.h"
#include <WiFi.h>


static const char *TAG = "ROBONOMICS_L1_WIFI";

// #########################################
// ### Read WiFi credentials             ###
// ### from NVS or setup Web Server, if  ###
// ### no credentials found              ###
// #########################################

void get_wifi_creds() {
    esp_err_t res;
    char ssid_buffer[100];
    char password_buffer[100];
    res = read_string_from_nvs(WIFI_SSID_NVS_KEY, ssid_buffer, sizeof(ssid_buffer));
    if (res == ESP_OK) {
        res = read_string_from_nvs(WIFI_PASSWORD_NVS_KEY, password_buffer, sizeof(ssid_buffer));
        if (res == ESP_OK) {
            user_data.ssid = String(ssid_buffer);
            user_data.password = String(password_buffer);
        }
    }
    if (res != ESP_OK) {
        set_wifi_setup_screen();
        user_data = get_wifi_creds_from_user();
        save_string_to_nvs(WIFI_SSID_NVS_KEY, user_data.ssid.c_str());
        save_string_to_nvs(WIFI_PASSWORD_NVS_KEY, user_data.password.c_str());
    }
}

// ##########################
// ### Read Battery state ###
// ##########################

unsigned int battery_level;

static void battery_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1) {
        battery_level = getBatteryState();
        ESP_LOGI(TAG, "Battery: %d", battery_level);
        set_lcd_battery(battery_level);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ######################
// ### Update display ###
// ######################

static void lcd_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}

extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    setupBatteryMeter();
    RGB_Init();
    RGB_Example();
    LCD_Init();
    BK_Light(50);
    LVGL_Init();
    set_brightness(30);
    xTaskCreate(lcd_task, "LCD_task", 4096, NULL, 5, NULL);
    set_wifi_setup_screen();
    xTaskCreate(battery_task, "Battery_Task", 2048, NULL, 7, NULL);
    get_wifi_creds();
    uint8_t counter = 0;
    set_connecting_screen(user_data.ssid.c_str());
    WiFi.begin(user_data.ssid, user_data.password);
    while ( WiFi.status() != WL_CONNECTED ) {
        vTaskDelay(500 /portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "." );
        counter++;
        if (counter > 60) {
            break;
        }
    }
    if ( WiFi.status() != WL_CONNECTED ) {
        set_not_connected_screen();
        vTaskDelay(5000 /portTICK_PERIOD_MS);
        nvs_flash_erase();
        esp_restart();
    } else {
        set_connected_screen();
    }
}