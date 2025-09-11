# ESP32-S3_ELRS_joystick

I programmed a device that receives data from the ELRS remote control via the CRSF protocol, processes it and transmits it to the computer. The computer receives input from the remote control as input from a joystick. This can be useful if you want to connect the remote control wirelessly, but your computer does not have bluetooth


The program consists of two classes: CRSF and gamepad:

🔹 In the CRSF class, in the void config_CRSF(int pin_tx, int pin_rx) method, we connect to the ELRS receiver via the UART protocol. In the function parameters, we pass the pins that will act as RX and TX. By default, these are pins 4 and 5.

🔹 In the gamepad class, in the void config_gamepad() method, the device is initialized as a gamepad. In the int32_t map_stick(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) method, one of the CRSF channels is received and scaled to the required values ​​​​for HID report. in_min and in_max indicate what the maximum and minimum values ​​​​can be data from the channel. out_min and out_max specify the maximum and minimum values ​​to which the data from the channel should be scaled.
Then the data from the channels is transmitted via the function void send_gamepad_report(...)



ESP32-S3 and the receiver should be connected to the following pins:
| ESP32-S3 | Receiver |
|----------|----------|
| GND      | GND      |
| 3V3      | Vin      |
| 4        | TX       |
| 5        | RX       |



Link for 3D model of case that you can print on 3D printer:
https://www.printables.com/model/1411700-esp32-s3-elrs-receiver-sase
