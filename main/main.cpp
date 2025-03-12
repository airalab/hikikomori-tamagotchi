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


// #if !defined ZB_ED_ROLE
// #error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
// #endif

static const char *TAG = "ESP_ZB_ON_OFF_LIGHT";

Robonomics robonomics;
int lcd_brightness = 30;
uint16_t buttonPressCount = 0;
int work_timeout = 5 * 60 * 1000000; // 5 min

esp_timer_handle_t sleep_mode_timer;


static void send_datalog_counter() {
    char message[50]; // Buffer to hold the resulting string
    sprintf(message, "Button was pressed %u times", buttonPressCount);
    ESP_LOGI(TAG, "Start sending Datalog: %s", message);
    buttonPressCount = 0;
    ESP_LOGI(TAG, "Connecting to %s", user_data.ssid.c_str());
    WiFi.begin(user_data.ssid, user_data.password);
    while ( WiFi.status() != WL_CONNECTED ) {
        vTaskDelay(500 /portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "." );
    }
    ESP_LOGI(TAG, "WiFi connected");
    robonomics.setup(user_data.host_url.c_str());
    const char* res = robonomics.sendRWSDatalogRecord(message, user_data.owner_address.c_str());
    ESP_LOGI(TAG, "Datalog result: %s", res);
    robonomics.disconnectWebsocket();
    WiFi.disconnect(true);
    set_datalog_sent_screen(res);
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
            res = read_string_from_nvs(OWNER_ADDRESS_NVS_KEY, owner_adddress_buffer, sizeof(owner_adddress_buffer));
            if (res == ESP_OK) {
                res = read_string_from_nvs(HOST_URL_NVS_KEY, host_url, sizeof(host_url));
                if (res == ESP_OK) {
                    user_data.ssid = String(ssid_buffer);
                    user_data.password = String(password_buffer);
                    user_data.owner_address = String(owner_adddress_buffer);
                    user_data.host_url = String(host_url);
                }
            }
        }
    }
    if (res != ESP_OK) {
        set_wifi_setup_screen();
        user_data = get_wifi_creds_from_user(robonomics.getSs58Address());
        save_string_to_nvs(WIFI_SSID_NVS_KEY, user_data.ssid.c_str());
        save_string_to_nvs(WIFI_PASSWORD_NVS_KEY, user_data.password.c_str());
        save_string_to_nvs(OWNER_ADDRESS_NVS_KEY, user_data.owner_address.c_str());
        save_string_to_nvs(HOST_URL_NVS_KEY, user_data.host_url.c_str());
    }
}

static void battery_task(void *pvParameters) {
    unsigned int battery_level;
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1) {
        battery_level = getBatteryState();
        ESP_LOGI(TAG, "Battery: %d", battery_level);
        set_lcd_battery(&battery_level);
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

void setup_button() {
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
}

void button_short_pressed_handle() {
    reset_sleep_timer();
    buttonPressCount++;
    if (!is_current_screen_main()) {
        set_main_screen();
    }
    set_lcd_counter(&buttonPressCount);
}

void go_to_sleep_mode() {
    ESP_LOGI(TAG, "Go to sleep mode");
    RGB_turn_off();
    lcd_brightness = 0;
    set_brightness(lcd_brightness);
    esp_deep_sleep_start();
}

void handle_sleep_timer(void* arg) {
    go_to_sleep_mode();
}

void button_long_pressed_handle() {
    stop_sleep_timer();
    set_sending_datalog_screen();
    send_datalog_counter();
    vTaskDelay(pdMS_TO_TICKS(1000));
    start_sleep_timer();
}


void reset_device_handle() {
    ESP_ERROR_CHECK(nvs_flash_erase());
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

void button_task(void *pvParameters) {
    int button_level = 1;
    int button_level_prev = 1;
    int button_pressed_count = 0;
    bool button_handled = false;
    int short_press_count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        button_level = gpio_get_level(BUTTON_GPIO);
        if (button_level == 0) {
            button_pressed_count++;
            if (button_pressed_count > 200) {
                ESP_LOGI(TAG, "Send Datalog");
                short_press_count = 0;
                button_long_pressed_handle();
                button_pressed_count = 0;
            }
        } else if (button_level_prev == 0) {
            if (button_pressed_count > 15) {
                short_press_count = 0;
                button_short_pressed_handle();
            } else if (button_pressed_count > 3) {
                short_press_count++;
            } else {
                short_press_count = 0;
            }
            if (short_press_count > 1) {
                go_to_sleep_mode();
            }
            button_handled = false;
            button_pressed_count = 0;
        }
        button_level_prev = button_level;
    }
}

void reset_sleep_timer() {
    esp_timer_restart(sleep_mode_timer, work_timeout);
}

void stop_sleep_timer() {
    esp_timer_stop(sleep_mode_timer);
}

void start_sleep_timer() {
    ESP_ERROR_CHECK(esp_timer_start_once(sleep_mode_timer, work_timeout));
}

void setup_sleep_mode_timer() {
    const esp_timer_create_args_t sleep_mode_timer_args = {
            .callback = &handle_sleep_timer,
            .name = "sleep-mode-timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&sleep_mode_timer_args, &sleep_mode_timer));
    start_sleep_timer();
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
    setup_button();
    setupBatteryMeter();
    RGB_Init();
    RGB_Example();
    LCD_Init();
    BK_Light(50);
    LVGL_Init();
    set_brightness(30);
    // esp_coex_wifi_i154_enable();
    xTaskCreate(lcd_task, "LCD_task", 4096, NULL, 5, NULL);
    xTaskCreate(button_task, "Button_Task", 16384, NULL, 6, NULL);
    xTaskCreate(battery_task, "Battery_Task", 2048, NULL, 7, NULL);
    setup_sleep_mode_timer();
    get_wifi_creds();
    reset_sleep_timer();
    set_main_screen();
}