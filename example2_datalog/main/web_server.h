#include <Arduino.h>

#define AP_SSID "Hikkikomory-Tamagotchi"

struct UserData {
    String ssid;
    String password;
    String host_url;
};

extern UserData user_data;

UserData get_wifi_creds_from_user(const char* robonomicsSs58Address);
