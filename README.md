# ESP32-S3-STD2

ESP32-S3-STD2 is a closed-loop controller based on the ESP32-S3 series. A TMC2209 dev board can be connected to control a stepper motor. An on-board encoder provides feedback for precise position control and an addressable RGB LED adds a visual feature for debugging. The board can be attached to the back of a Nema 17 Stepper motor.<br>

<img src='/PCB_TopView.JPG' width='350'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src='/PCB_BottomView.JPG' width='350'><br>


&nbsp;<br>
### ESP32-S3-WROOM-1-N4 MCU

The ESP32-S3-WROOM-1-N4 is an MCU with WiFi 802.11n and Bluetooth LE capabilities. It has a full-speed USB 2.0 OTG which reduces the amount of components needed.<br>


&nbsp;<br>
### AS5600L Hall-effect Encoder

The AS5600L is a magnetic rotary position sensor and is used with a radial magnet. This sensor has a high-resolution 12-bit I²C or PWM output. The AS5600L can be used for precise position control of the stepper motor.<br>


&nbsp;<br>
### WS2812B Addressable LED

WS2812B is an addressable LED that has a control circuit and RGB chip integrated. This visual feature is useful for debugging and testing the basic functionalities of the circuit board.<br>


&nbsp;<br>
### TMC2209 Stepper Driver
Headers are present on the board to connect a TMC2209 dev board or an equivalent. ESP32-S3-STD3 has been successfully tested with a BigTreeTech TMC2209 V1.3 board. This module can be controlled with UART.<br>


&nbsp;<br>
### AZ1117 3.3V Regulator
This SMD 3.3V regulator powers the ESP32 and the other components. The board is powered by either the V-BUS (5Vdc) of the USB-C or the VCC connection for the motor (Maximum 15Vdc). If both supplies are connected at the same time, the highest voltage rules.<br>
