#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "esp_log.h"
#include "driver/adc.h"

static const char *TAG = "usb_gamepad";

#define DAED_ZONE 7

#define NUM_BUTTONS 14

const gpio_num_t button_pins[NUM_BUTTONS] = {
    GPIO_NUM_38, //X button
    GPIO_NUM_37,
    GPIO_NUM_36,  
    GPIO_NUM_35,  
    GPIO_NUM_6,  
    GPIO_NUM_7,
    GPIO_NUM_1,  
    GPIO_NUM_9,  
    GPIO_NUM_10, 
    GPIO_NUM_11,
    GPIO_NUM_8, //L3 button
    GPIO_NUM_3, //R3 button
    GPIO_NUM_12,
    GPIO_NUM_13
};


#define REPORT_ID_GAMEPAD  4

static const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD))
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

static void send_gamepad_report(int8_t x_in, int8_t y_in, int8_t zX_in, int8_t zY_in, uint16_t  buttons_in) {
hid_gamepad_report_t report = {
    .x = x_in,
    .y = y_in,
    .z = zX_in,
    .rz = zY_in,
    .hat = 0,         
    .buttons = buttons_in
};

tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

int32_t map_stick(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    int r = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if((r<=DAED_ZONE)&&(r>=-DAED_ZONE)){
        return 0;
    }
    return r;
}

void my_configure_buttons() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_config_t cfg = {
            .pin_bit_mask = 1ULL << button_pins[i],
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&cfg);
    }
}

uint16_t read_buttons() {
    uint16_t buttons = 0;
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (gpio_get_level(button_pins[i]) == 0) {
            buttons |= (1 << i);
        }
    }
    return buttons;
}

void app_main(void) {

    my_configure_buttons();
    adc2_config_channel_atten(ADC2_CHANNEL_4, ADC_ATTEN_DB_11); // GPIO15
    adc2_config_channel_atten(ADC2_CHANNEL_5, ADC_ATTEN_DB_11); // GPIO16
    adc2_config_channel_atten(ADC2_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO17
    adc2_config_channel_atten(ADC2_CHANNEL_7, ADC_ATTEN_DB_11); // GPIO18

    int raw15 = 0, raw16 = 0, raw17 = 0, raw18 = 0;

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB Gamepad ready");

    int8_t x =0, y = 0, zX = 0, zY = 0;

    while (1) {
        if (tud_mounted()) {
            uint16_t  buttons = read_buttons();
             if (adc2_get_raw(ADC2_CHANNEL_4, ADC_WIDTH_BIT_12, &raw15) == ESP_OK &&
            adc2_get_raw(ADC2_CHANNEL_5, ADC_WIDTH_BIT_12, &raw16) == ESP_OK &&
            adc2_get_raw(ADC2_CHANNEL_6, ADC_WIDTH_BIT_12, &raw17) == ESP_OK &&
            adc2_get_raw(ADC2_CHANNEL_7, ADC_WIDTH_BIT_12, &raw18) == ESP_OK
            ) {
            x = map_stick(raw15, 0, 4095, -127, 127);
            y = map_stick(raw16, 0, 4095, -127, 127);
            zX = map_stick(raw17, 0, 4095, -127, 127);
            zY = map_stick(raw18, 0, 4095, -127, 127);
            }
            send_gamepad_report(x, y, zX, zY,buttons);
        }
    }
}