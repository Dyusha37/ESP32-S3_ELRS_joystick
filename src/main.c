#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_log.h"
#include "driver/adc.h"

#include "ESP_CRSF.h"
#define PIN_RX 11
#define PIN_TX 12

static const char *gamepad_TAG = "usb_gamepad";

#define DAED_ZONE 0

#define MAP_MAX 2047
#define MAP_MIN 0

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
    /* 8 bit DPad/Hat Button Map  */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                 ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_HAT_SWITCH           ) ,\
    HID_LOGICAL_MIN    ( 1                                      ) ,\
    HID_LOGICAL_MAX    ( 8                                      ) ,\
    HID_PHYSICAL_MIN   ( 0                                      ) ,\
    HID_PHYSICAL_MAX_N ( 315, 2                                 ) ,\
    HID_REPORT_COUNT   ( 1                                      ) ,\
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

static const uint8_t hid_report_descriptor[] = {
    MY_TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD))
};

const char *hid_string_descriptor[] = {
    (const char[]){0x09, 0x04},  // LangID: English
    "TinyUSB",                   // Manufacturer
    "ESP32-S3 Gamepad",          // Product
    "123456",                    // Serial
    "Gamepad Interface"          // Interface
};

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

const uint8_t hid_configuration_descriptor[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, 0, 100),
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 8, 10)
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    return hid_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize) {
}



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
  uint8_t hat;       ///< Buttons mask for currently pressed buttons in the DPad/hat
  uint32_t buttons;  ///< Buttons mask for currently pressed buttons
}my_hid_gamepad_report_t;

static void send_gamepad_report(int8_t x_in, int8_t y_in, int8_t zX_in, int8_t zY_in, int8_t zL_in, int8_t zR_in,int8_t zL1_in, int8_t zR1_in, uint16_t  buttons_in) {
my_hid_gamepad_report_t report = {
    .x = x_in,
    .y = y_in,
    .z = zX_in,
    .rz = zY_in,
    .rx = zL_in,
    .ry = zR_in,
    .slider1 = zL1_in,
    .slider2 = zR1_in,
    .hat = 0,         
    .buttons = buttons_in
};

tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

int32_t map_stick(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    int r = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if((r<=DAED_ZONE)&&(r>=-DAED_ZONE))
        { return 0; }
    return r; 
}


void app_main(void) {
    crsf_config_t config = {
    .uart_num = UART_NUM_1,
    .tx_pin = PIN_TX,
    .rx_pin = PIN_RX
    };
    CRSF_init(&config);
    crsf_channels_t channels = {0};

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(gamepad_TAG, "USB Gamepad ready");

    int8_t x =0, y = 0, zX = 0, zY = 0, zL =0, zR =0,zL1 =0, zR1 =0;

    while (1) {
        if (tud_mounted()) {
        uint16_t  buttons = 0;
        CRSF_receive_channels(&channels);

        x =  map_stick(channels.ch1, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        y =  map_stick(channels.ch2, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zX = map_stick(channels.ch3, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zY = map_stick(channels.ch6, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zL = map_stick(channels.ch4, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zR = map_stick(channels.ch5, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zL1 = map_stick(channels.ch7, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        zR1 = map_stick(channels.ch8, MAP_MIN, MAP_MAX, OUT_MIN, OUT_MAX);
        if (channels.ch9 >= 1024 ) {
            buttons |= (1 << 0);
        }
        if (channels.ch10 >= 1024 ) {
            buttons |= (1 << 1);
        }
        send_gamepad_report(x, y, zX, zY, zL, zR, zL1, zR1,buttons);
        }
    }
}