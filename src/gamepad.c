#include "gamepad.h"

const char *gamepad_TAG = "usb_gamepad";

static const uint8_t hid_report_descriptor[];

const char *hid_string_descriptor[] = {
    (const char[]){0x09, 0x04},  // LangID: English
    "TinyUSB",                   // Manufacturer
    "ESP32-S3 Gamepad",          // Product
    "123456",                    // Serial
    "Gamepad Interface"          // Interface
};


static const uint8_t hid_report_descriptor[] = {
    MY_TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD))
};


uint8_t hid_configuration_descriptor[] = {
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


void config_gamepad(){
        const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(gamepad_TAG, "USB Gamepad ready");

}

int32_t map_stick(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    int r = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if((r<=DAED_ZONE)&&(r>=-DAED_ZONE))
        { return 0; }
    return r; 
}

void send_gamepad_report(int8_t x_in, int8_t y_in, int8_t zX_in, int8_t zY_in, int8_t zL_in, int8_t zR_in,int8_t zL1_in, int8_t zR1_in, uint16_t  buttons_in) {
    my_hid_gamepad_report_t report = {
    .x = x_in,
    .y = y_in,
    .z = zX_in,
    .rz = zY_in,
    .rx = zL_in,
    .ry = zR_in,
    .slider1 = zL1_in,
    .slider2 = zR1_in, 
    .buttons = buttons_in
    };
tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}



