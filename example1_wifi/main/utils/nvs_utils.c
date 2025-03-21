// #include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_utils.h"

static const char *TAG = "NVS_UTILS";

void write_blob_to_nvs(const char *key, const uint8_t *data, size_t data_length) {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(ROBONOMICS_STORAGE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Write data %s to flash", data);
        err = nvs_set_blob(my_handle, key, data, data_length);
        ESP_LOGI(TAG, "%s", err != ESP_OK ? "Failed!" : "Done");
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        ESP_LOGI(TAG, "%s", err != ESP_OK ? "Failed!" : "Done");
    }
    nvs_close(my_handle);
}

esp_err_t read_blob_from_nvs(const char *key, uint8_t *private_key, size_t data_length) {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(ROBONOMICS_STORAGE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err; 
    }
    ESP_LOGI(TAG, "Reading %s from flash", key);
    size_t required_size;
    err = nvs_get_blob(my_handle, key, NULL, &required_size);
    
    if (err == ESP_OK) {
        if (required_size == data_length) {
            err = nvs_get_blob(my_handle, key, private_key, &data_length);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "%s was read successfully", key);
            } else {
                ESP_LOGI(TAG, "Failed to read data: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "Data size mismatch! Expected: %d, Found: %d", data_length, required_size);
            err = ESP_ERR_NVS_INVALID_LENGTH;
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "The value is not initialized yet!");
    } else {
        ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(err));
    }

    nvs_close(my_handle);
    return err;
}
void save_string_to_nvs(const char *key, const char *value) {
    nvs_handle my_handle;
    ESP_LOGI(TAG, "Write data %s to flash key %s", value, key);
    esp_err_t err = nvs_open(ROBONOMICS_STORAGE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        err = nvs_set_str(my_handle, key, value);
        if (err == ESP_OK) {
            err = nvs_commit(my_handle);
            ESP_LOGI(TAG, "Data %s was succesfully written to flash key %s", value, key);
        } else {
            ESP_LOGI(TAG, "Failed to save string data: %s", esp_err_to_name(err));
        }
        nvs_close(my_handle);
    }
}

esp_err_t read_string_from_nvs(const char *key, char *buffer, size_t buffer_size) {
    nvs_handle my_handle;
    ESP_LOGI(TAG, "Reading %s from flash", key);
    esp_err_t err = nvs_open(ROBONOMICS_STORAGE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        size_t required_size;
        err = nvs_get_str(my_handle, key, NULL, &required_size);
        if (err == ESP_OK && required_size <= buffer_size) {
            err = nvs_get_str(my_handle, key, buffer, &required_size);
            ESP_LOGI(TAG, "%s was read successfully: %s", key, buffer);
        } else {
            ESP_LOGI(TAG, "Failed to read string data: %s", esp_err_to_name(err));
        }
        nvs_close(my_handle);
    }
    return err;
}

void save_int_to_nvs(const char* key, int value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ROBONOMICS_STORAGE, NVS_READWRITE, &handle);
    ESP_LOGI(TAG, "Write data %d to flash key %s", value, key);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle! %s", esp_err_to_name(err));
    } else {
        err = nvs_set_i32(handle, key, value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Data %d was succesfully written to flash key %s", value, key);
        } else {
            ESP_LOGE(TAG, "Error saving value to NVS! %s", esp_err_to_name(err));
        }

        err = nvs_commit(handle); // Commit the changes
        nvs_close(handle); // Close the NVS handle
    }
}

esp_err_t read_int_from_nvs(const char* key, int* value) {
    nvs_handle_t handle;
    ESP_LOGI(TAG, "Reading %s from flash", key);
    esp_err_t err = nvs_open(ROBONOMICS_STORAGE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle! %s", esp_err_to_name(err));
        return err;
    }

    size_t required_size;
    err = nvs_get_i32(handle, key, value);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Value read from %s key flash: %d", key, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key not found!");
        *value = 0; // Default value if not found
    } else {
        ESP_LOGE(TAG, "Error reading value from NVS! %s", esp_err_to_name(err));
    }

    nvs_close(handle); // Close the NVS handle
    return err;
}