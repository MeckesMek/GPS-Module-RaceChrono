# MeckesMek GPS Module for RaceChrono

MeckesMek GPS unit for RaceChrono (Pro Version) – get it here: <https://racechrono.com>

Perfect for motocross/Enduro, Folkrace, RallyCross etc.

Read more about the GPS unit in the User Manual PDF. It covers how to connect to RaceChrono, update firmware, etc.

---

## MechTrack

MechTrack is an Arduino-based firmware add-on that combines GPS position/speed data with real-time mechanical sensor readings and streams everything to the **RaceChrono** app over Bluetooth.

### Features

| Channel | Description | Unit |
|---------|-------------|------|
| GPS | Latitude, longitude, altitude, speed, heading, satellites | — |
| RPM | Engine RPM (interrupt-based pulse counter) | rev/min |
| Coolant temp | Engine coolant temperature (NTC thermistor) | °C |
| Oil temp | Engine oil temperature (NTC thermistor) | °C |
| Oil pressure | Engine oil pressure (resistive / 0–5 V sensor) | bar |
| Throttle | Throttle position sensor (TPS) | % |
| Battery | Battery voltage (resistive divider) | V |
| Alert LED | Lights up when any threshold is exceeded | — |

### Hardware requirements

- Arduino Uno / Nano (or compatible 5 V board)
- NMEA GPS module (e.g. u-blox NEO-6M) – communicates over SoftwareSerial
- HC-05 or HC-06 Bluetooth module – communicates over SoftwareSerial
- NTC 10 kΩ thermistors for coolant and oil temperature
- Resistive or 0–5 V oil pressure sensor
- Throttle position sensor (0–5 V)
- Resistive voltage divider for battery voltage (e.g. 30 kΩ / 10 kΩ)

### Default wiring

| Signal | Arduino pin |
|--------|-------------|
| GPS TX | 4 (SoftwareSerial RX) |
| GPS RX | 5 (SoftwareSerial TX) |
| BT TX  | 6 (SoftwareSerial RX) |
| BT RX  | 7 (SoftwareSerial TX) |
| RPM signal | 2 (INT0) |
| Coolant NTC | A0 |
| Oil temp NTC | A1 |
| Oil pressure | A2 |
| Throttle TPS | A3 |
| Battery divider | A4 |
| Alert LED | 13 (built-in) |

Pin assignments, alert thresholds and sensor calibration constants can all be changed in [`MechTrack/config.h`](MechTrack/config.h).

### Installation

1. Open `MechTrack/MechTrack.ino` in the Arduino IDE (1.8 or later).
2. Select your board and COM port.
3. Click **Upload**.

No additional libraries are required beyond **SoftwareSerial**, which is bundled with the Arduino IDE.

### Enabling debug output

Add `#define MECHTRACK_DEBUG` before the `#include` lines in `MechTrack.ino` (or pass `-DMECHTRACK_DEBUG` as a build flag) to mirror all sensor data to the hardware serial port at 115 200 baud.

### RaceChrono setup

1. In the RaceChrono app go to **Settings → OBD-II / DIY channels**.
2. Add a new Bluetooth device and select **MechTrack** from the paired devices list.
3. Add custom channels using the channel IDs defined in `config.h` (`RC_CHANNEL_RPM`, `RC_CHANNEL_COOLANT`, etc.).

### License

MIT – see repository root for details.
