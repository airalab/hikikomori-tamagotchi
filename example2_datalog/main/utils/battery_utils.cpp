#include <SparkFunBQ27441.h>
#include "esp_log.h"

// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 80;

static const char *TAG = "BATTERY_UTILS";

void setupBatteryMeter(void)
{
  if (!lipo.begin()) { // begin() will return true if communication is successful
    ESP_LOGI(TAG, "Error: Unable to communicate with BQ27441.");
  }
  ESP_LOGI(TAG, "Connected to BQ27441!");
  lipo.setCapacity(BATTERY_CAPACITY);
}

unsigned int getBatteryState() {
    return lipo.soc();
}
