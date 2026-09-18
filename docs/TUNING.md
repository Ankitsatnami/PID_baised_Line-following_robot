# PID Tuning Guide

1. Fix mechanics first: equal wheels, straight sensor bar, symmetric motors, low center of mass, good traction and minimal backlash.

2. Start with BASE_SPEED around 100-130 and KI = 0.

3. Increase KP until tracking becomes responsive and just begins to oscillate. Back KP down slightly.

4. Increase KD until the oscillation is damped and the robot tracks smoothly.

5. Add a very small KI only for persistent bias. Too much KI causes delayed correction and overshoot.

6. Increase BASE_SPEED in small steps. Test the hardest curve after every change.

The firmware dynamically lowers speed as line error grows, so straight sections can run faster than sharp turns.

If the OLED sensor bar looks inverted, change BLACK_IS_HIGH. If left/right is reversed, change SENSOR_REVERSED.

If the robot drifts consistently, check wheel alignment first, then use LEFT_TRIM or RIGHT_TRIM for small compensation.

Before high speed, verify no Nano reset/brownout, adequate battery voltage, stable sensor readings and safe motor/driver temperature.
