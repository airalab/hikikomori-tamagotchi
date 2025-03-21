#include "LVGL_Example.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "images/party_image.c"
#include "images/persevering_face.c"

static const char *TAG = "ESP_ZB_ON_OFF_LIGHT";

#define BACKLIGHT_PIN 22
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_TIMER   LEDC_TIMER_0
#define LEDC_MODE    LEDC_LOW_SPEED_MODE
#define LEDC_FREQUENCY 5000
#define LEDC_RESOLUTION LEDC_TIMER_8_BIT

static lv_obj_t *label_battery; // Battery level label
lv_obj_t *label_counter;
uint8_t current_screen_id = 0;
unsigned int last_battery_lvl = 100;

LV_IMG_DECLARE(party_popper_1f389);
LV_IMG_DECLARE(persevering_face);

bool is_current_screen_main() {
    return current_screen_id == 1;
}

void set_lcd_battery(unsigned int v) {
    last_battery_lvl = v;
    char buf[20];
    if (last_battery_lvl > 80) snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_FULL" %u%%", v); 
    else if (last_battery_lvl < 80 && last_battery_lvl > 60) snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_3" %u%%", v);
    else if (last_battery_lvl < 60 && last_battery_lvl > 40) snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_2" %u%%", v); 
    else if (last_battery_lvl < 40 && last_battery_lvl > 20) snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_1" %u%%", v); 
    else if (last_battery_lvl < 20) snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_EMPTY" %u%%", v); 
    lv_label_set_text(label_battery, buf);
}

void set_lcd_counter(uint16_t* v) {
    char buf[6];
    snprintf(buf, sizeof(buf), "%u", *v); 
    lv_label_set_text(label_counter, buf);
}

void set_brightness(int brightness) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_RESOLUTION,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = BACKLIGHT_PIN,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

// Common function to create a battery label on the top-right corner
void create_battery_label(lv_obj_t *parent) {
    label_battery = lv_label_create(parent);
    lv_obj_align(label_battery, LV_ALIGN_TOP_RIGHT, -10, 10);
    set_lcd_battery(last_battery_lvl);
}

#define LV_SYMBOL_PARTY_POPPER   "\xF0\x9F\x8E\x89"

// Screen 1: Big number in the center
void set_connected_screen() {
    ESP_LOGI(TAG, "Connacted Screen");
    current_screen_id = 1;
    
    lv_obj_t *screen = lv_obj_create(NULL);
    
    // Create label for the message
    lv_obj_t *label_message = lv_label_create(screen);
    lv_obj_t *img_emoji = lv_img_create(screen);
    lv_img_set_src(img_emoji, &party_popper_1f389);
    lv_img_set_zoom(img_emoji, 250);
    lv_obj_align(img_emoji, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_text(label_message, "Connected to WiFi!");
    lv_obj_align(label_message, LV_ALIGN_CENTER, 0, -30); // Centered

    // Create battery label
    create_battery_label(screen);
    
    lv_scr_load(screen);
}

void set_not_connected_screen() {
    ESP_LOGI(TAG, "Not Connected Screen");
    current_screen_id = 2;
    
    lv_obj_t *screen = lv_obj_create(NULL);
    
    // Create label for the message
    lv_obj_t *label_message = lv_label_create(screen);
    lv_obj_t *img_emoji = lv_img_create(screen);
    lv_img_set_src(img_emoji, &persevering_face);
    lv_img_set_zoom(img_emoji, 250);
    lv_obj_align(img_emoji, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_text(label_message, "Can't connect to WiFi...");
    lv_obj_align(label_message, LV_ALIGN_CENTER, 0, -30); // Centered

    // Create battery label
    create_battery_label(screen);
    
    lv_scr_load(screen);
}

#define MAX_DOTS 3  // Maximum number of dots

static lv_obj_t *label_message;  // Label for "Connecting to <ssid>"
static lv_obj_t *label_dots;     // Label for dots
static int dot_count = 0;        // Current dot count


static void update_dots(lv_timer_t *timer) {
    static char dots_str[MAX_DOTS + 2] = "";  // Buffer for dots (e.g., "...")

    dot_count = (dot_count + 1) % (MAX_DOTS + 1);  // Cycle between 0 and MAX_DOTS
    memset(dots_str, '.', dot_count);  // Fill with dots
    dots_str[dot_count] = '\0';  // Null-terminate the string

    lv_label_set_text(label_dots, dots_str);  // Update label
}


void set_connecting_screen(const char *ssid) {
    lv_obj_t *screen = lv_obj_create(NULL);

    label_message = lv_label_create(screen);
    static char message[64];  // Buffer to store the formatted string
    snprintf(message, sizeof(message), "Connecting to %s", ssid);
    lv_label_set_text(label_message, message);
    lv_obj_align(label_message, LV_ALIGN_CENTER, 0, -20);  // Center the message

    label_dots = lv_label_create(screen);
    lv_label_set_text(label_dots, "");  // Initially empty
    lv_obj_align(label_dots, LV_ALIGN_CENTER, 0, 10);  // Place below message

    lv_timer_create(update_dots, 500, NULL);

    lv_scr_load(screen);
}

// Screen 2: "Setup WiFi info"
void set_wifi_setup_screen() {
    ESP_LOGI(TAG, "Setup WiFi Screen");
    current_screen_id = 4;
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_t *label_text = lv_label_create(screen);
    lv_label_set_text(label_text, "Setup WiFi info");
    // lv_obj_set_style_text_font(label_text, &lv_font_montserrat_28, 0);
    lv_obj_center(label_text);
    create_battery_label(screen);
    lv_scr_load(screen);
}


