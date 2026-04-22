# Night Fury — RC Plane with Custom Flight Controller

A handbuilt foam RC airplane with a custom stabilization system built from scratch using an Arduino Mega and MPU-6050 IMU.

![Night Fury RC Plane](photos/night_fury.jpg)

## What it does

The Night Fury is a fixed-wing RC airplane that I designed and built entirely by hand from foamboard. It uses a custom flight controller I wrote from scratch that reads roll and pitch angles from an IMU and automatically corrects the control surfaces to keep the plane stable — making it significantly easier to fly.

## Hardware

- Handmade foamboard airframe
- Brushless tractor motor + ESC
- 2200mAh 3S LiPo battery
- 4x micro servos (ailerons, elevator, rudder)
- Arduino Mega 2560
- MPU-6050 IMU (I2C)
- FlySky receiver (i-Bus protocol)
- 3D printed motor mount (custom CAD)

## How the flight controller works

The Arduino reads gyroscope and accelerometer data from the MPU-6050 via I2C. A complementary filter fuses both sensors to get stable roll and pitch angles. On startup, the system calibrates the gyroscope automatically by averaging 200 samples. The stabilization loop runs at ~50Hz and blends the pilot's radio inputs with automatic corrections — the pilot controls direction, the FC keeps it level.

## Firmware

Written in Arduino C++. Key features:
- Complementary filter (gyro + accelerometer fusion)
- Gyroscope auto-calibration on boot
- i-Bus receiver parsing (FlySky protocol)
- PWM servo output for ailerons, elevator, and rudder
- ESC throttle control via PWM

## CAD

The motor mount was designed in SolidWorks and 3D printed. Source files are in the `/cad` folder.

## Build status

- [x] Airframe construction
- [x] Electronics integration  
- [x] IMU reading + complementary filter
- [x] Servo stabilization working
- [ ] i-Bus receiver integration (in progress)
- [ ] Flight test with FC onboard
- [ ] PCB design (planned)

## Flight tests

6 flight attempts — 2 successful flights of ~10 meters each. Main challenge was pilot erro
