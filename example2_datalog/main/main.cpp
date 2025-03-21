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
#include <Ed25519.h>
#include "Robonomics.h"
#include "utils/nvs_utils.h"
#include "address.h"
#include "LCD_Driver/ST7789.h"
#include "RGB/RGB.h"
#include "LVGL_UI/LVGL_Example.h"
#include "driver/gpio.h"
#include "utils/battery_utils.h"
#include <cJSON.h>


static const char *TAG = "ESP_ZB_ON_OFF_LIGHT";

Robonomics robonomics;

const char* datalog_data = "Hello, World!!!";

static void send_datalog() {
    const char* res = robonomics.sendDatalogRecord(datalog_data);
    ESP_LOGI(TAG, "Datalog result: %s", res);
    robonomics.disconnectWebsocket();
    WiFi.disconnect(true);
    if (res[0] == '0' && res[1] == 'x') {
        set_datalog_sent_screen(res);
    } else {
        cJSON *json = cJSON_Parse(res);
        if (json == NULL) {
            ESP_LOGI(TAG, "Error parsing JSON\n");
                return;
        }

        // Get the "data" field
        cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
        if (cJSON_IsString(data) && (data->valuestring != NULL)) {
            ESP_LOGI(TAG, "Extracted Data: %s\n", data->valuestring);
            set_datalog_not_sent_screen(data->valuestring);
        } else {
            ESP_LOGI(TAG, "No valid 'data' field found\n");
            cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "message");
            if (cJSON_IsString(data) && (data->valuestring != NULL)) {
                ESP_LOGI(TAG, "Extracted Data: %s\n", data->valuestring);
                set_datalog_not_sent_screen(data->valuestring);
            } else {
                ESP_LOGI(TAG, "No valid 'message' field found\n");
            }
        }

        cJSON_Delete(json);
    }
}

void get_or_generate_private_key() {
    esp_err_t ret;
    char private_key[100];
    ret = read_string_from_nvs(ROBONOMICS_PRIVATE_KEY_NVS_KEY, private_key, sizeof(private_key));
    if (ret == ESP_OK) {
        robonomics.setPrivateKey(private_key);
        ESP_LOGI(TAG, "Robonomics Address: %s", robonomics.getSs58Address());
    } else {
        // Ed25519::generatePrivateKey(robonomicsPrivateKey);
        robonomics.generateAndSetPrivateKey();
        save_string_to_nvs(ROBONOMICS_PRIVATE_KEY_NVS_KEY, robonomics.getPrivateKey());
        ESP_LOGI(TAG, "Robonomics Address: %s", robonomics.getSs58Address());
    }
}

void get_wifi_creds() {
    esp_err_t res;
    char ssid_buffer[100];
    char password_buffer[100];
    char owner_adddress_buffer[100];
    char host_url[100];
    res = read_string_from_nvs(WIFI_SSID_NVS_KEY, ssid_buffer, sizeof(ssid_buffer));
    if (res == ESP_OK) {
        res = read_string_from_nvs(WIFI_PASSWORD_NVS_KEY, password_buffer, sizeof(ssid_buffer));
        if (res == ESP_OK) {
            res = read_string_from_nvs(HOST_URL_NVS_KEY, host_url, sizeof(host_url));
            if (res == ESP_OK) {
                user_data.ssid = String(ssid_buffer);
                user_data.password = String(password_buffer);
                user_data.host_url = String(host_url);
            }
        }
    }
    if (res != ESP_OK) {
        set_wifi_setup_screen();
        user_data = get_wifi_creds_from_user(robonomics.getSs58Address());
        save_string_to_nvs(WIFI_SSID_NVS_KEY, user_data.ssid.c_str());
        save_string_to_nvs(WIFI_PASSWORD_NVS_KEY, user_data.password.c_str());
        save_string_to_nvs(HOST_URL_NVS_KEY, user_data.host_url.c_str());
    }
}

static void battery_task(void *pvParameters) {
    unsigned int battery_level;
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1) {
        battery_level = getBatteryState();
        ESP_LOGI(TAG, "Battery: %d", battery_level);
        set_lcd_battery(battery_level);
        esp_timer_get_time();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void lcd_task(void *pvParameters) {
    float angle_float;
    int32_t angle;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}

static void datalog_task(void *pvParameters) {
    send_datalog();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
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
    get_or_generate_private_key();
    setupBatteryMeter();
    RGB_Init();
    RGB_Example();
    LCD_Init();
    BK_Light(50);
    LVGL_Init();
    set_brightness(30);
    xTaskCreate(lcd_task, "LCD_task", 4096, NULL, 5, NULL);
    xTaskCreate(battery_task, "Battery_Task", 2048, NULL, 7, NULL);
    get_wifi_creds();
    robonomics.setup(user_data.host_url.c_str());
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
    vTaskDelay(3000 /portTICK_PERIOD_MS);
    set_sending_datalog_screen(datalog_data);
    xTaskCreate(datalog_task, "Datalog_Task", 16384, NULL, 6, NULL);
}