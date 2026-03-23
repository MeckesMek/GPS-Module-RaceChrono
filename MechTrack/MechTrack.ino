/*
 * MechTrack – GPS + mechanical data tracker for RaceChrono
 * =========================================================
 * Reads GPS position/speed/altitude from a NMEA GPS module
 * and mechanical sensor data (RPM, temperatures, oil pressure,
 * throttle position and battery voltage) from analog/digital
 * inputs.  All data are transmitted over Bluetooth to the
 * RaceChrono app (https://racechrono.com) at a configurable
 * rate.
 *
 * Hardware wiring summary
 * -----------------------
 *  GPS module TX  → Arduino pin 4  (GPS_RX in software)
 *  GPS module RX  → Arduino pin 5  (GPS_TX in software – optional)
 *  BT  module TX  → Arduino pin 6  (BT_RX  in software)
 *  BT  module RX  → Arduino pin 7  (BT_TX  in software)
 *  RPM signal     → Arduino pin 2  (INT0 – interrupt capable)
 *  Coolant NTC    → Arduino pin A0
 *  Oil temp NTC   → Arduino pin A1
 *  Oil pressure   → Arduino pin A2
 *  Throttle TPS   → Arduino pin A3
 *  Battery divider→ Arduino pin A4
 *  Alert LED      → Arduino pin 13 (built-in LED)
 *
 * See config.h to customise pin assignments, alert thresholds
 * and sensor calibration constants.
 *
 * Dependencies (install via Arduino Library Manager)
 * --------------------------------------------------
 *  • SoftwareSerial  (bundled with the Arduino IDE)
 *
 * Author : MeckesMek
 * License: MIT
 */

#include <Arduino.h>
#include "config.h"
#include "gps_handler.h"
#include "mechanical_data.h"
#include "bluetooth_handler.h"

// ---- Pin assignments for SoftwareSerial instances ---------------
static const uint8_t GPS_RX_PIN = 4;
static const uint8_t GPS_TX_PIN = 5;
static const uint8_t BT_RX_PIN  = 6;
static const uint8_t BT_TX_PIN  = 7;

// ---- Module instances -------------------------------------------
GpsHandler      gps(GPS_RX_PIN, GPS_TX_PIN);
MechanicalData  mech;
BluetoothHandler bt(BT_RX_PIN, BT_TX_PIN);

// ---- Arduino entry points ---------------------------------------

void setup() {
    Serial.begin(115200);
    Serial.println(F("MechTrack starting…"));

    gps.begin();
    mech.begin();
    bt.begin();

    Serial.println(F("MechTrack ready."));
}

void loop() {
    gps.update();
    mech.update();
    bt.update(gps.data(), mech.data());

    // Optional: mirror data to the hardware serial port for debugging.
#ifdef MECHTRACK_DEBUG
    const GpsData&  g = gps.data();
    const MechData& m = mech.data();

    Serial.print(F("GPS valid="));  Serial.print(g.valid);
    Serial.print(F(" lat="));       Serial.print(g.latitude,  6);
    Serial.print(F(" lon="));       Serial.print(g.longitude, 6);
    Serial.print(F(" spd="));       Serial.print(g.speed_kmh, 1);
    Serial.print(F(" alt="));       Serial.print(g.altitude_m, 1);
    Serial.print(F(" hdg="));       Serial.print(g.heading_deg, 1);
    Serial.print(F(" sats="));      Serial.println(g.satellites);

    Serial.print(F("MECH rpm="));   Serial.print(m.rpm);
    Serial.print(F(" cool="));      Serial.print(m.coolant_temp, 1);
    Serial.print(F(" oil_t="));     Serial.print(m.oil_temp, 1);
    Serial.print(F(" oil_p="));     Serial.print(m.oil_pressure, 2);
    Serial.print(F(" tps="));       Serial.print(m.throttle_pct, 1);
    Serial.print(F(" batt="));      Serial.print(m.battery_v, 2);
    Serial.print(F(" alert="));     Serial.println(m.alert);
#endif
}
