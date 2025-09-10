#include "gamepad.h"
#include "CRSF.h"

#define PIN_RX 4
#define PIN_TX 5

void app_main(void) {

    config_gamepad();
    config_CRSF(PIN_TX, PIN_RX);
    
    crsf_channels_t channels = {0};

    int8_t x =0, y = 0, zX = 0, zY = 0, zL =0, zR =0,zL1 =0, zR1 =0;

    while (1) {
        if (tud_mounted()) {
        uint16_t  buttons = 0;
        CRSF_receive_channels(&channels);
        x =  map_stick(channels.ch1, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        y =  map_stick(channels.ch2, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zX = map_stick(channels.ch3, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zY = map_stick(channels.ch6, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zL = map_stick(channels.ch4, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zR = map_stick(channels.ch5, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zL1 = map_stick(channels.ch7, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
        zR1 = map_stick(channels.ch8, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX);
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