#include "CRSF.h"

void config_CRSF(int pin_tx, int pin_rx){
    crsf_config_t config = {
    .uart_num = UART_NUM_1,
    .tx_pin = pin_tx,
    .rx_pin = pin_rx
    };
    CRSF_init(&config);
}
