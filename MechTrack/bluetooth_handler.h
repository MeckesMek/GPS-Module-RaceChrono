#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "gps_handler.h"
#include "mechanical_data.h"

// ============================================================
// Bluetooth Handler
// Serialises GPS and mechanical data into the simple text
// protocol understood by RaceChrono's "DIY channel" feature
// and streams it over a SoftwareSerial link to an HC-05/HC-06
// Bluetooth module.
//
// Protocol (one line per update):
//   $MT,<time_ms>,<lat>,<lon>,<speed_kmh>,<alt_m>,<heading>,
//       <sats>,<rpm>,<coolant>,<oil_temp>,<oil_press>,
//       <throttle>,<batt>*<CRC>\r\n
//
// RaceChrono custom channels are also sent using the standard
// OBD-II/PID-style single-value frames so they appear as
// separate channels in the app.
// ============================================================

class BluetoothHandler {
public:
    BluetoothHandler(uint8_t rx_pin, uint8_t tx_pin)
        : _serial(rx_pin, tx_pin), _last_tx_ms(0)
    {}

    void begin() {
        _serial.begin(BT_BAUD_RATE);
    }

    // Call from loop().  Transmits one frame per TRANSMIT_INTERVAL_MS.
    void update(const GpsData& gps, const MechData& mech) {
        unsigned long now = millis();
        if (now - _last_tx_ms < TRANSMIT_INTERVAL_MS) return;
        _last_tx_ms = now;

        _sendCombinedFrame(gps, mech, now);
        _sendChannelFrames(mech);
    }

private:
    SoftwareSerial _serial;
    unsigned long  _last_tx_ms;

    // ---- Combined GPS + mechanical frame ----------------------------
    void _sendCombinedFrame(const GpsData& gps, const MechData& mech,
                            unsigned long time_ms) {
        // Resolve GPS fields once; use zeros when no fix is available.
        double lat = gps.valid ? (double)gps.latitude    : 0.0;
        double lon = gps.valid ? (double)gps.longitude   : 0.0;
        double spd = gps.valid ? (double)gps.speed_kmh   : 0.0;
        double alt = gps.valid ? (double)gps.altitude_m  : 0.0;
        double hdg = gps.valid ? (double)gps.heading_deg : 0.0;

        char buf[200];
        int  len = snprintf(buf, sizeof(buf),
            "$MT,%lu,%.6f,%.6f,%.2f,%.1f,%.1f,%u,"
            "%u,%.1f,%.1f,%.2f,%.1f,%.2f",
            time_ms,
            lat, lon, spd, alt, hdg,
            (unsigned)gps.satellites,
            (unsigned)mech.rpm,
            (double)mech.coolant_temp,
            (double)mech.oil_temp,
            (double)mech.oil_pressure,
            (double)mech.throttle_pct,
            (double)mech.battery_v);

        if (len > 0 && len < (int)sizeof(buf)) {
            uint8_t crc = _calcCrc(buf + 1, len - 1); // skip leading '$'
            char    footer[8];
            snprintf(footer, sizeof(footer), "*%02X\r\n", crc);
            _serial.print(buf);
            _serial.print(footer);
        }
    }

    // ---- Per-channel frames (RaceChrono DIY channel protocol) -------
    // Format: $RC,<channel_id>,<value>*<CRC>\r\n
    void _sendChannelFrames(const MechData& mech) {
        _sendChannel(RC_CHANNEL_RPM,       mech.rpm);
        _sendChannel(RC_CHANNEL_COOLANT,   mech.coolant_temp);
        _sendChannel(RC_CHANNEL_OIL_TEMP,  mech.oil_temp);
        _sendChannel(RC_CHANNEL_OIL_PRESS, mech.oil_pressure);
        _sendChannel(RC_CHANNEL_THROTTLE,  mech.throttle_pct);
        _sendChannel(RC_CHANNEL_BATTERY,   mech.battery_v);
    }

    void _sendChannel(uint8_t channel_id, float value) {
        char buf[48];
        int  len = snprintf(buf, sizeof(buf),
                            "$RC,%u,%.2f", (unsigned)channel_id, (double)value);
        if (len > 0 && len < (int)sizeof(buf)) {
            uint8_t crc = _calcCrc(buf + 1, len - 1);
            char    footer[8];
            snprintf(footer, sizeof(footer), "*%02X\r\n", crc);
            _serial.print(buf);
            _serial.print(footer);
        }
    }

    // XOR checksum of all bytes between (but not including) '$' and '*'.
    static uint8_t _calcCrc(const char* data, int len) {
        uint8_t crc = 0;
        for (int i = 0; i < len; i++) {
            crc ^= static_cast<uint8_t>(data[i]);
        }
        return crc;
    }
};

#endif // BLUETOOTH_HANDLER_H
