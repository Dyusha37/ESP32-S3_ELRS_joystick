#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_log.h"
#include "driver/adc.h"

#define DAED_ZONE 0

#define OUT_MAX 127
#define OUT_MIN -127

#define MY_TUD_HID_REPORT_DESC_GAMEPAD(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                 ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    /* 8 bit X, Y, Z, Rz, Rx, Ry (min -127, max 127 ) */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                 ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_X                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Y                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Z                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RZ                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RX                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RY                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_SLIDER ), \
    HID_USAGE          ( HID_USAGE_DESKTOP_SLIDER ), \
    HID_LOGICAL_MIN    ( OUT_MIN                                   ) ,\
    HID_LOGICAL_MAX    ( OUT_MAX                                   ) ,\
    HID_REPORT_COUNT   ( 8                                     ) ,\
    HID_REPORT_SIZE    ( 8                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* 32 bit Button Map */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON                  ) ,\
    HID_USAGE_MIN      ( 1                                      ) ,\
    HID_USAGE_MAX      ( 32                                     ) ,\
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    HID_LOGICAL_MAX    ( 1                                      ) ,\
    HID_REPORT_COUNT   ( 32                                     ) ,\
    HID_REPORT_SIZE    ( 1                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END \

#define REPORT_ID_GAMEPAD  4

typedef struct TU_ATTR_PACKED
{
  int8_t  x;         ///< Delta x  movement of left analog-stick
  int8_t  y;         ///< Delta y  movement of left analog-stick
  int8_t  z;         ///< Delta z  movement of right analog-joystick
  int8_t  rz;        ///< Delta Rz movement of right analog-joystick
  int8_t  rx;        ///< Delta Rx movement of analog left trigger
  int8_t  ry;        ///< Delta Ry movement of analog right trigger
  int8_t slider1;       
  int8_t slider2;
  uint32_t buttons;  ///< Buttons mask for currently pressed buttons
}my_hid_gamepad_report_t;


#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) ;
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize);


void config_gamepad();
int32_t map_stick(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
void send_gamepad_report(int8_t x_in, int8_t y_in, int8_t zX_in, int8_t zY_in, int8_t zL_in, int8_t zR_in,int8_t zL1_in, int8_t zR1_in, uint16_t  buttons_in);


#endif