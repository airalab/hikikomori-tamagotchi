set(COMPONENT_REQUIRES "Crypto" "Arduino_JSON" "WebSockets" "mbedtls" "lvgl__lvgl" "SparkFun_BQ27441" "esp-robonomics-client" "RGB_Driver" "LVGL_Driver" "LCD_Driver")

idf_component_register(SRCS "main.cpp"
                       "LVGL_UI/LVGL_Example.c"
                       "web_server.cpp"
                       "utils/nvs_utils.c"
                       "utils/battery_utils.cpp"
                    INCLUDE_DIRS ".")