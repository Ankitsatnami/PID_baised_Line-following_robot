# PID Biased High-Speed Line Following Robot

> Arduino Nano + TB6612FNG + 8-Channel IR + 74HC4051 + SSD1306 OLED

A high-speed, smooth and tunable PID line-following robot focused on fast response, stable steering, dynamic speed control and practical tuning.

## Project Status

Current stage: Hardware + firmware foundation (V1).

The firmware currently assumes an 8-channel analog IR sensor array. If the actual sensor module uses digital, serial or another interface, the sensor driver must be adapted.

## Design Goals

- High speed on straight sections
- Smooth steering
- Accurate line tracking
- Automatic speed reduction in sharp turns
- Controlled line-loss recovery
- Easy PID tuning
- Live OLED telemetry
- Stable operation under motor load

## System Architecture

8-channel IR sensor array -> 74HC4051 -> Arduino Nano -> PID -> TB6612FNG -> Left/Right N20 motors

Arduino Nano also handles OLED telemetry and push buttons.

## Hardware

| Component | Qty | Purpose |
|---|---:|---|
| Arduino Nano | 1 | Main controller |
| TB6612FNG | 1 | Dual motor driver |
| 6V 600RPM N20 motor | 2 | Drive |
| 8-channel analog IR sensor | 1 | Line detection |
| 74HC4051 | 1 | Analog multiplexer |
| SSD1306 I2C OLED | 1 | Telemetry |
| Push buttons | 3 | Start / calibration / speed |
| SPDT switch | 1 | Main power |
| Battery | 1 | Motor + logic power |
| 5V buck/regulator | 1 | Stable logic supply |
| 100uF electrolytic | 1+ | Motor supply smoothing |
| 0.1uF ceramic | 1+ | Supply decoupling |
| 0.1uF motor capacitors | 2 | Optional noise suppression |

## Power Architecture

IMPORTANT: Never power the motors from the Arduino Nano 5V pin.

Recommended power path:

Battery -> SPDT switch -> TB6612 VM

Battery -> 5V buck/regulator -> Nano 5V + IR sensor + 74HC4051 + OLED

All grounds must be common.

Place a 100uF electrolytic and 0.1uF ceramic close to the TB6612 VM/GND supply. Optional 0.1uF capacitors can be fitted directly across the motor terminals.

## Pin Configuration

### TB6612FNG -> Arduino Nano

| TB6612FNG | Nano |
|---|---|
| PWMA | D5 |
| AIN1 | D7 |
| AIN2 | D8 |
| PWMB | D6 |
| BIN1 | D9 |
| BIN2 | D10 |
| STBY | D4 |
| VCC | 5V |
| VM | Motor supply |
| GND | Common GND |

A01/A02 -> Left motor

B01/B02 -> Right motor

If one motor rotates in the wrong direction, swap that motor's two wires.

### 74HC4051

| 74HC4051 | Arduino Nano |
|---|---|
| SIG / Z | A0 |
| S0 / A | D11 |
| S1 / B | D12 |
| S2 / C | D13 |
| EN | GND |
| VCC | 5V |
| GND | Common GND |
| X0-X7 | IR channels 0-7 |

Physical sensor order expected by firmware: LEFT -> RIGHT.

Sensor positions are approximately -3500, -2500, -1500, -500, +500, +1500, +2500, +3500.

If the physical sensor order is reversed, change SENSOR_REVERSED to true in the firmware.

### OLED

| OLED | Nano |
|---|---|
| SDA | A4 |
| SCL | A5 |
| VCC | Module-rated supply |
| GND | Common GND |

Default firmware I2C address: 0x3C.

### Push Buttons

| Function | Nano | Connection |
|---|---|---|
| START | D2 | D2 -> button -> GND |
| MODE/CAL | D3 | D3 -> button -> GND |
| SPEED - | A1 | A1 -> button -> GND |

The firmware uses Arduino internal pull-ups.

## PID Controller

Correction = KP x Error + KI x Integral + KD x Derivative

The controller includes:

- Derivative low-pass filtering
- Integral limiting and decay to reduce windup
- Dynamic speed reduction as tracking error increases
- PWM slew limiting to avoid abrupt motor commands
- Controlled line-loss search

Conceptually:

Small error -> high speed

Medium error -> reduced speed

Large error -> lower speed

Line lost -> controlled search

## Starting PID Values

KP = 0.105

KI = 0.0008

KD = 0.82

These are starting values only. Final values depend on the chassis, motors, wheels, sensor height, track, battery and line characteristics.

## PID Tuning Procedure

1. Fix the mechanics first: equal wheels, straight sensor bar, rigid chassis, symmetric motor mounting and good traction.
2. Start around BASE_SPEED 100-130 with KI = 0.
3. Increase KP until tracking is responsive and just begins to oscillate; then reduce it slightly.
4. Increase KD until oscillation is damped and steering becomes smooth.
5. Add a very small KI only if persistent bias remains.
6. Increase BASE_SPEED in small steps and test the hardest curve after each change.

Do not use PID to hide mechanical problems.

## High-Speed Strategy

Straight -> high speed -> curve approach -> error increases -> dynamic speed reduction -> smooth correction -> line centered -> accelerate again.

The intention is to use high speed where the robot has enough tracking margin instead of driving maximum PWM everywhere.

## OLED Telemetry

The OLED displays run/stop state, position error, PID constants, motor PWM, a compact sensor visualization, speed and integral information.

## Line-Loss Recovery

When the total sensor signal becomes too low, the firmware treats the line as lost and searches in the direction of the previous line position. The search is deliberately controlled rather than an immediate full-power spin.

## Motor Compensation

Small motor differences can be compensated with LEFT_TRIM and RIGHT_TRIM. Check wheel alignment, wheel diameter, motor mounting and wiring before using large trim values.

## Testing Procedure

### Test 1 - Wheels lifted
Verify motor direction, TB6612 operation, OLED, buttons, sensor readings and calibration.

### Test 2 - Slow straight
Verify sensor order, PID direction and motor polarity.

### Test 3 - Slow curves
Tune KP and KD.

### Test 4 - Higher speed
Increase BASE_SPEED gradually.

### Test 5 - Hardest track section
Optimize dynamic speed and derivative response.

## Troubleshooting

Robot steers opposite: check sensor order, motor polarity and SENSOR_REVERSED.

Robot oscillates: reduce KP, increase KD slightly, reduce speed, and check sensor height/noise.

Robot reacts too slowly: increase KP carefully and verify sensor sampling and sensor height.

Robot suddenly spins: check sensor order, line-loss threshold, excessive PID correction and motor polarity.

Nano resets during acceleration: check battery current capability, buck converter, ground wiring, TB6612 supply wiring and bulk capacitors.

## Repository Structure

PID_baised_Line-following_robot/

README.md

src/PID_LineFollower.ino

docs/WIRING.md

docs/TUNING.md

## Roadmap

- [ ] Confirm exact IR sensor module
- [ ] Optimize sensor sampling for the actual board
- [ ] Improve sensor noise filtering
- [ ] Adaptive PID
- [ ] Predictive / preview steering
- [ ] Advanced dynamic speed control
- [ ] OLED configuration menu
- [ ] Button-based KP/KI/KD adjustment
- [ ] EEPROM PID profile storage
- [ ] Multiple track profiles
- [ ] Motor characterization
- [ ] Battery-voltage monitoring
- [ ] Start-line detection
- [ ] Lap/timing support
- [ ] High-speed corner optimization

## Project

PID Biased High-Speed Line Following Robot

Built around Arduino Nano + TB6612FNG + 8CH IR + 74HC4051 + SSD1306.

High speed. Smooth control. Precise tracking.