set(COMPONENT_REQUIRES "Crypto" "Arduino_JSON" "WebSockets" "mbedtls" "lvgl__lvgl" "SparkFun_BQ27441")

idf_component_register(SRCS "main.cpp"
                       "LCD_Driver/Vernon_ST7789T/Vernon_ST7789T.c"
                       "LCD_Driver/ST7789.c"
                       "LVGL_Driver/LVGL_Driver.c"
                       "LVGL_UI/LVGL_Example.c"
                       "RGB/RGB.c"
                       "robonomics_utils/address.cpp"
                       "web_server.cpp"
                       "robonomics_utils/blake/blake2b.cpp"
                       "utils/nvs_utils.c"
                       "robonomics_utils/BlockchainUtils.cpp"
                       "robonomics_utils/Call.cpp"
                       "robonomics_utils/Encoder.cpp"
                       "robonomics_utils/Extrinsic.cpp"
                       "robonomics_utils/JsonUtils.cpp"
                       "robonomics_utils/PayloadParamsUtils.cpp"
                       "robonomics_utils/Robonomics.cpp"
                       "robonomics_utils/Utils.cpp"
                       "robonomics_utils/WebsocketUtils.cpp"
                       "utils/battery_utils.cpp"
                    INCLUDE_DIRS ".")