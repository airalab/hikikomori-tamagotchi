#ifdef __cplusplus
extern "C" {
#endif

#include "esp_check.h"

#define ROBONOMICS_STORAGE "robonomics"
#define ROBONOMICS_PRIVATE_KEY_NVS_KEY "priv_key"
#define WIFI_SSID_NVS_KEY "wifi_ssid"
#define WIFI_PASSWORD_NVS_KEY "wifi_password"
#define HAPPINESS_NVS_KEY "happiness"
#define OWNER_ADDRESS_NVS_KEY "owner_address"
#define HOST_URL_NVS_KEY "host_url"

void write_blob_to_nvs(const char *key, const uint8_t *data, size_t data_length);
esp_err_t read_blob_from_nvs(const char *key, uint8_t *private_key, size_t data_length);
void save_string_to_nvs(const char *key, const char *value);
esp_err_t read_string_from_nvs(const char *key, char *buffer, size_t buffer_size);
void save_int_to_nvs(const char* key, int value);
esp_err_t read_int_from_nvs(const char* key, int* value);

#ifdef __cplusplus
}
#endif