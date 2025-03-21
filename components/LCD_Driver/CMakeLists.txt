cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "Vernon_ST7789T/Vernon_ST7789T.c"
                       "ST7789.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer esp_lcd driver lvgl__lvgl
                    PRIV_REQUIRES driver
                    )

project(LCD_Driver)