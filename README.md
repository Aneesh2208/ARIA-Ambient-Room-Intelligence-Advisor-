# ARIA — Ambient Room Intelligence Advisor

A basic room environment monitor built on Arduino UNO using components from the PES University 3rd semester ECE starter kit.

---

## What it does

- Reads temperature, humidity, air quality and ambient light continuously
- Displays live readings across 4 rotating screens on a 16x2 LCD
- Wakes up when you walk in (PIR motion detection), sleeps after 1 minute of no movement
- Scrolls an alert message and sounds a buzzer when conditions cross comfortable limits
- Detects when room lights are turned on or off and announces it
- Gives the room a score out of 10 based on temperature, humidity and air quality
- Prints all events and readings to Serial Monitor for logging

---

## Hardware

| Component | Role | Arduino Pin |
|---|---|---|
| Arduino UNO | Main controller | — |
| DHT11 | Temperature + humidity | D2 |
| MQ135 | Air quality (relative, uncalibrated) | A0 |
| LDR module | Ambient light level | A1 |
| PIR sensor | Occupancy / motion detection | D3 |
| 16x2 LCD (I2C) | Display | A4 (SDA), A5 (SCL) |
| 5V Buzzer | Audio alerts | D8 |
| Breadboard | Power distribution (5V + GND rails) | — |

---

## Wiring

All VCC pins share the breadboard 5V rail, connected from Arduino 5V.  
All GND pins share the breadboard GND rail, connected from Arduino GND.  
Signal wires connect directly from each component to the corresponding Arduino pin.

---

## Alert thresholds

| Condition | Limit | Basis |
|---|---|---|
| Temperature | > 32°C | WHO indoor comfort guideline |
| Humidity | > 60% | WHO recommended indoor range (30–60%) |
| Air quality | > 600 (raw sensor value) | Relative threshold, not calibrated AQI |

**Note on air quality:** The MQ135 outputs a raw analog value from 0–1023 based on gas concentration. This is not a calibrated AQI reading — it is a relative indicator. The categories (Good / Moderate / Poor) reflect sensor output ranges, not official air quality standards. Proper AQI calculation requires sensor-specific calibration curves which have not been applied here.

---

## Alert sounds

| Condition | Buzzer pattern |
|---|---|
| Poor air quality | 3 long beeps |
| Too hot and humid | 2 medium beeps |
| Too hot only | 1 beep |
| Too humid only | 1 beep |
| Lights turned on | 2 short quick beeps |
| Lights turned off | 1 long beep |
| Display wake on motion | 1 short beep |

Alerts repeat at most once every 5 minutes to avoid constant noise.

---

## LCD screens

Cycles every 3 seconds while display is active:
```
Screen 1              Screen 2              Screen 3              Screen 4
T:32.1C H:54%         Air Quality:          Light: 726            Room Score:
Comfortable           Good (89)             Brightness level      8/10  Good
```

---

## Serial Monitor output

On boot:
```
ARIA starting...
Calibrating sensors...
Alert limits: Temp 32C | Humidity 60% | Air Quality 600
Boot readings — Temp: 32.4C  Humidity: 63.00%  Air Quality: 90
WARNING: Room already above humidity limit at boot
```

During operation:
```
INFO: Motion detected - display on
Temp: 32.1C  Humidity: 63.00%  Air Quality: 90  Light: 726
ALERT: Too hot and humid
INFO: Lights turned off
INFO: No motion - display off
```

---

## Libraries required

Install via Arduino IDE Library Manager:

- DHT sensor library — Adafruit
- LiquidCrystal I2C — Frank de Brabander
- Keypad — Mark Stanley, Alexander Brevig (installed, not used in current version)

---

## Known limitations

- MQ135 requires 24–48 hours of continuous power on first use to fully stabilise
- No real-time clock — device has no awareness of time of day
- Light detection threshold (500) may need adjustment depending on room and sensor placement
- Air quality reading is relative, not calibrated to any standard

---

## Built with

Arduino IDE 2.3.8  
PES University ECE Starter Kit — Combo C
