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
#include "freertos/task.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "web_server.h"
#include "esp_coexist.h"
#include <Ed25519.h>
#include "robonomics_utils/Robonomics.h"
#include "utils/nvs_utils.h"
#include "robonomics_utils/address.h"


#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile light (End Device) source code.
#endif

static const char *TAG = "ESP_ZB_ON_OFF_LIGHT";

Robonomics robonomics;
float happiness = 0;
// WiFiCredentials wifi_creds;

esp_err_t esp_zcl_utility_add_ep_basic_manufacturer_info(esp_zb_ep_list_t *ep_list, uint8_t endpoint_id, zcl_basic_manufacturer_info_t *info)
{
    esp_err_t ret = ESP_OK;
    esp_zb_cluster_list_t *cluster_list = NULL;
    esp_zb_attribute_list_t *basic_cluster = NULL;

    cluster_list = esp_zb_ep_list_get_ep(ep_list, endpoint_id);
    ESP_RETURN_ON_FALSE(cluster_list, ESP_ERR_INVALID_ARG, TAG, "Failed to find endpoint id: %d in list: %p", endpoint_id, ep_list);
    basic_cluster = esp_zb_cluster_list_get_cluster(cluster_list, ESP_ZB_ZCL_CLUSTER_ID_BASIC, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    ESP_RETURN_ON_FALSE(basic_cluster, ESP_ERR_INVALID_ARG, TAG, "Failed to find basic cluster in endpoint: %d", endpoint_id);
    ESP_RETURN_ON_FALSE((info && info->manufacturer_name), ESP_ERR_INVALID_ARG, TAG, "Invalid manufacturer name");
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, info->manufacturer_name));
    ESP_RETURN_ON_FALSE((info && info->model_identifier), ESP_ERR_INVALID_ARG, TAG, "Invalid model identifier");
    ESP_ERROR_CHECK(esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, info->model_identifier));
    return ret;
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_RETURN_ON_FALSE(esp_zb_bdb_start_top_level_commissioning(mode_mask) == ESP_OK, , TAG, "Failed to start Zigbee commissioning");
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p       = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    // esp_zb_app_signal_type_t sig_type = *p_sg_p;
    esp_zb_app_signal_type_t sig_type = static_cast<esp_zb_app_signal_type_t>(*p_sg_p);
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            // ESP_LOGI(TAG, "Deferred driver initialization %s", deferred_driver_init() ? "failed" : "successful");
            ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted");
            }
        } else {
            /* commissioning failed */
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}

static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    if (message->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT) {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                ESP_LOGI(TAG, "Light sets to %s", light_state ? "On" : "Off");
                // light_driver_set_power(light_state);
            }
        }
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID) {
                if (happiness < 100) {
                    happiness += HAPPINESS_INCREASE_STEP;
                }
                ESP_LOGI(TAG, "Identify message, happiness: %.2f%%", happiness);
                // light_driver_set_power(light_state);
            }
        }
    }
    return ret;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

static void esp_zb_task(void *pvParameters)
{
    /* initialize Zigbee stack */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);
    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *esp_zb_on_off_light_ep = esp_zb_on_off_light_ep_create(HA_ESP_LIGHT_ENDPOINT, &light_cfg);
    zcl_basic_manufacturer_info_t info = {
        .manufacturer_name = ESP_MANUFACTURER_NAME,
        .model_identifier = ESP_MODEL_IDENTIFIER,
    };

    esp_zcl_utility_add_ep_basic_manufacturer_info(esp_zb_on_off_light_ep, HA_ESP_LIGHT_ENDPOINT, &info);
    esp_zb_device_register(esp_zb_on_off_light_ep);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));
    while (true)
    {
        esp_zb_stack_main_loop_iteration();
    }
}

static void send_datalog_happiness_full() {
    WiFi.begin(user_data.ssid, user_data.password);
    while ( WiFi.status() != WL_CONNECTED ) {
        vTaskDelay(500 /portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "." );
    }
    robonomics.setup();
    robonomics.sendRWSDatalogRecord("Happiness is 100%%!", user_data.owner_address.c_str());
    robonomics.disconnectWebsocket();
    WiFi.disconnect(true);
}

static void happiness_manage_task(void *pvParameters) {
    uint16_t counter_decrease = 0;
    uint16_t counter_save = 0;
    float happiness_prev = 0;
    while (true)
    {
        vTaskDelay(1000 /portTICK_PERIOD_MS);
        if (happiness >= 100 && happiness_prev < 100) {
            send_datalog_happiness_full();
        }
        counter_decrease++;
        counter_save++;
        if (counter_decrease > HAPPINESS_DECREASE_INTERVAL) {
            counter_decrease = 0;
            happiness -= HAPPINESS_DECREASE_STEP;
            ESP_LOGI(TAG, "Happiness decreased: %.2f%%", happiness);
        }
        if (counter_save > HAPPINESS_SAVE_INTERVAL) {
            counter_save = 0;
            save_int_to_nvs(HAPPINESS_NVS_KEY, (int)happiness);
        }
        happiness_prev = happiness;
    }
}

void get_or_generate_private_key(uint8_t *robonomicsPrivateKey) {
    char* robonomicsSs58Address;
    esp_err_t ret;
    ret = read_blob_from_nvs(ROBONOMICS_PRIVATE_KEY_NVS_KEY, robonomicsPrivateKey, sizeof(robonomicsPrivateKey));
    if (ret == ESP_OK) {
        robonomicsSs58Address = getAddrFromPrivateKey(robonomicsPrivateKey);
        ESP_LOGI(TAG, "Robonomics Address: %s", robonomicsSs58Address);
    } else {
        Ed25519::generatePrivateKey(robonomicsPrivateKey);
        write_blob_to_nvs(ROBONOMICS_PRIVATE_KEY_NVS_KEY, robonomicsPrivateKey, sizeof(robonomicsPrivateKey));
        robonomicsSs58Address = getAddrFromPrivateKey(robonomicsPrivateKey);
        ESP_LOGI(TAG, "Robonomics Address: %s", robonomicsSs58Address);
    }
    delete[] robonomicsSs58Address;
}

void get_wifi_creds() {
    esp_err_t res;
    char ssid_buffer[100];
    char password_buffer[100];
    char owner_adddress_buffer[100];
    res = read_string_from_nvs(WIFI_SSID_NVS_KEY, ssid_buffer, sizeof(ssid_buffer));
    if (res == ESP_OK) {
        res = read_string_from_nvs(WIFI_PASSWORD_NVS_KEY, password_buffer, sizeof(ssid_buffer));
        if (res == ESP_OK) {
            res = read_string_from_nvs(OWNER_ADDRESS_NVS_KEY, owner_adddress_buffer, sizeof(owner_adddress_buffer));
            if (res == ESP_OK) {
                user_data.ssid = String(ssid_buffer);
                user_data.password = String(password_buffer);
                user_data.owner_address = String(owner_adddress_buffer);
            }
        }
    }
    if (res != ESP_OK) {
        user_data = get_wifi_creds_from_user(robonomics.getSs58Address());
        save_string_to_nvs(WIFI_SSID_NVS_KEY, user_data.ssid.c_str());
        save_string_to_nvs(WIFI_PASSWORD_NVS_KEY, user_data.password.c_str());
        save_string_to_nvs(OWNER_ADDRESS_NVS_KEY, user_data.owner_address.c_str());
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
    uint8_t robonomicsPrivateKey[PRIVATE_KEY_LENGTH];
    int happiness_int;
    ret = read_int_from_nvs(HAPPINESS_NVS_KEY, & happiness_int);
    if (ret == ESP_OK) {
        happiness = happiness_int;
    } else {
        happiness = 0;
    }
    get_or_generate_private_key(robonomicsPrivateKey);
    robonomics.setPrivateKey(robonomicsPrivateKey);
    get_wifi_creds();
    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));
    xTaskCreate(esp_zb_task, "Zigbee_main", 8192, NULL, 5, NULL);
    esp_coex_wifi_i154_enable();
    xTaskCreate(happiness_manage_task, "Happiness_manage", 8192, NULL, 6, NULL);
    // xTaskCreate(happiness_manage_task, "Datalog_main", 8192, NULL, 7, NULL);
}