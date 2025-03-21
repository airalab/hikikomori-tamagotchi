cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "LVGL_Driver.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_timer lvgl__lvgl esp_lcd driver
                    )

project(LVGL_Driver)