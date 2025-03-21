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

#ifdef __cplusplus
extern "C" {
#endif



/* Tamagochi Configuration */
#define HAPPINESS_INCREASE_STEP 0.2
#define HAPPINESS_DECREASE_STEP 5
#define HAPPINESS_DECREASE_INTERVAL 3600 /* in seconds */
#define HAPPINESS_SAVE_INTERVAL 30 /* in seconds */

#define BUTTON_GPIO GPIO_NUM_9

/**
 * @brief Adds manufacturer information to the ZCL basic cluster of endpoint
 * 
 * @param[in] ep_list The pointer to the endpoint list with @p endpoint_id
 * @param[in] endpoint_id The endpoint identifier indicating where the ZCL basic cluster resides
 * @param[in] info The pointer to the basic manufacturer information
 * @return
 *      - ESP_OK: On success
 *      - ESP_ERR_INVALID_ARG: Invalid argument
 */
void reset_sleep_timer();
void stop_sleep_timer();
void start_sleep_timer();

#ifdef __cplusplus
}
#endif