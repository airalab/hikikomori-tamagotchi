cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS "RGB.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver led_strip
                    )

project(RGB_Driver)