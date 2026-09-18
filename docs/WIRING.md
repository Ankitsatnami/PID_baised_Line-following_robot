# Detailed Circuit Connection

## Power
Battery positive -> SPDT switch -> TB6612 VM.
Battery positive -> 5 V buck/regulator -> Nano 5V, IR VCC, 4051 VCC and OLED if supported.
Battery negative -> common GND.
Nano GND, driver GND, sensor GND, 4051 GND and OLED GND must be common.

Place 100 uF electrolytic and 0.1 uF ceramic close to TB6612 VM/GND. Optional 0.1 uF ceramic directly across each motor.

## TB6612FNG
PWMA -> Nano D5
AIN1 -> D7
AIN2 -> D8
PWMB -> D6
BIN1 -> D9
BIN2 -> D10
STBY -> D4
VCC -> 5V
VM -> switched motor supply
GND -> common GND
A01/A02 -> left motor
B01/B02 -> right motor

If forward commands make one motor spin backward, reverse that motor's two wires.

## 74HC4051
SIG/Z -> Nano A0
S0/A -> D11
S1/B -> D12
S2/C -> D13
VCC -> 5V
GND -> common GND
EN -> GND
X0-X7 -> IR analog channels 0-7

## 8-channel IR
This project assumes eight independent analog outputs.
AO0 -> X0, AO1 -> X1, ... AO7 -> X7.
VCC/GND as specified by the sensor board.

If the board is digital-only, this exact 4051 analog arrangement is not applicable.

## OLED SSD1306 I2C
SDA -> A4
SCL -> A5
VCC -> module-rated supply
GND -> common GND
Typical address is 0x3C.

## Buttons
START: D2 -> button -> GND
MODE: D3 -> button -> GND
MINUS: A1 -> button -> GND
Firmware uses INPUT_PULLUP.

## Mechanical setup
Keep sensor bar rigid and centered. A starting sensor height of about 5-10 mm is common, but use the distance specified by your sensor. Keep the array slightly ahead of the drive axle and keep wheels equal.

Keep motor wiring separate from sensor wiring. Use short sensor leads and a solid common ground.
