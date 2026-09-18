# PID Biased High-Speed Smooth Line Following Robot

Arduino Nano + TB6612FNG + 8-channel analog IR sensor + 74HC4051 + SSD1306 OLED.

## Goals
- Fast but controlled line tracking
- Smooth steering with filtered PID
- Dynamic speed reduction in sharp turns
- Integral anti-windup and derivative filtering
- Sensor calibration
- OLED telemetry

## Hardware
- Arduino Nano
- TB6612FNG
- 2 x 6 V 600 RPM N20 gear motors
- 8-channel analog IR sensor array
- 74HC4051 8:1 analog multiplexer
- 0.96 inch SSD1306 I2C OLED
- 3 push buttons
- SPDT switch
- Battery suitable for the motors and driver
- 5 V buck/regulator if needed
- 100 uF electrolytic + 0.1 uF ceramic near TB6612 VM
- Optional 0.1 uF ceramic across each motor

## Sensor assumption
This firmware assumes the 8-channel board exposes 8 independent analog outputs. They connect to the 74HC4051. The 4051 is needed because Nano A4/A5 are used by the I2C OLED.

## Pin map

### TB6612
PWMA D5, AIN1 D7, AIN2 D8
PWMB D6, BIN1 D9, BIN2 D10
STBY D4
VCC 5V, VM motor battery, GND common
A01/A02 left motor, B01/B02 right motor

### 74HC4051
SIG/Z A0
S0 D11, S1 D12, S2 D13
VCC 5V, GND common, EN GND
X0-X7 -> IR channels 0-7

### OLED
SDA A4, SCL A5, VCC appropriate module voltage, GND common

### Buttons
START D2 to GND
MODE D3 to GND
MINUS A1 to GND
All use internal pull-ups.

## Power
Do not power motors from the Nano 5 V pin.

Battery -> SPDT -> TB6612 VM
Battery -> 5 V buck/regulator -> Nano 5V + IR + 4051 + OLED

All grounds common. Put bulk capacitors close to TB6612.

## Firmware
Open src/PID_LineFollower.ino.

Required Arduino libraries:
- Adafruit GFX Library
- Adafruit SSD1306

## Calibration and tuning
Power on and move the sensor over representative white and black areas during calibration. Start with a conservative BASE_SPEED, then tune KP, KD, and finally a small KI.

The included values are starting points; chassis, sensor height, track and battery change the optimum settings.

## Mechanical tips
Keep the sensor bar rigid and low, wheels equal in diameter, chassis symmetric, center of mass low, and battery firmly mounted. Put the sensor array somewhat ahead of the drive axle for useful preview distance.

Bench-test with wheels lifted before high-speed track testing.
