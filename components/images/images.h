#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

LV_IMG_DECLARE(party_popper_1f389);
LV_IMG_DECLARE(persevering_face);

#ifdef __cplusplus
}
#endif