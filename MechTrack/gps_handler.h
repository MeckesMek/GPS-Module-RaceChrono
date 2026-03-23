#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"

// ============================================================
// GPS Handler
// Parses NMEA sentences from the GPS module and exposes the
// latest fix data through a simple GpsData struct.
// ============================================================

struct GpsData {
    bool    valid;          // True when a valid fix is available
    float   latitude;       // Decimal degrees (positive = North)
    float   longitude;      // Decimal degrees (positive = East)
    float   speed_kmh;      // Speed over ground in km/h
    float   altitude_m;     // Altitude above sea level in metres
    float   heading_deg;    // True course in degrees (0–360)
    uint8_t satellites;     // Number of satellites in view
    uint8_t hour;           // UTC hour   (from GPRMC)
    uint8_t minute;         // UTC minute
    uint8_t second;         // UTC second
};

class GpsHandler {
public:
    // rx_pin / tx_pin: SoftwareSerial pins wired to the GPS module.
    GpsHandler(uint8_t rx_pin, uint8_t tx_pin)
        : _serial(rx_pin, tx_pin), _buf_pos(0)
    {
        memset(&_data, 0, sizeof(_data));
        memset(_buf, 0, sizeof(_buf));
    }

    void begin() {
        _serial.begin(GPS_BAUD_RATE);
    }

    // Call from loop() to process incoming bytes.
    void update() {
        while (_serial.available()) {
            char c = static_cast<char>(_serial.read());
            if (c == '\n' || c == '\r') {
                if (_buf_pos > 0) {
                    _buf[_buf_pos] = '\0';
                    _parseSentence(_buf);
                    _buf_pos = 0;
                }
            } else if (static_cast<size_t>(_buf_pos) < sizeof(_buf) - 1) {
                _buf[_buf_pos++] = c;
            }
        }
    }

    const GpsData& data() const { return _data; }

private:
    SoftwareSerial _serial;
    GpsData        _data;
    char           _buf[128];
    int            _buf_pos;

    // ---- NMEA helpers ------------------------------------------------

    static float _parseLatLon(const char* field, const char* dir,
                               int deg_digits) {
        if (!field || field[0] == '\0') return 0.0f;
        // Degrees are the first deg_digits characters, minutes follow.
        char deg_str[4] = {0};
        strncpy(deg_str, field, deg_digits);
        float degrees = atof(deg_str);
        float minutes = atof(field + deg_digits);
        float result  = degrees + minutes / 60.0f;
        if (dir && (dir[0] == 'S' || dir[0] == 'W')) result = -result;
        return result;
    }

    // Returns pointer to the n-th comma-delimited field in a CSV string.
    // The returned pointer points into 'buf'; 'buf' is modified in place.
    static const char* _field(char* buf, int n) {
        char* p = buf;
        for (int i = 0; i < n; i++) {
            p = strchr(p, ',');
            if (!p) return "";
            p++;
        }
        char* end = strchr(p, ',');
        if (end) *end = '\0';
        return p;
    }

    // Parse a local copy of the sentence (modified by _field).
    void _parseSentence(const char* sentence) {
        // Validate checksum ($...*XX)
        if (sentence[0] != '$') return;
        const char* star = strchr(sentence, '*');
        if (star) {
            uint8_t calc = 0;
            for (const char* p = sentence + 1; p < star; p++) {
                calc ^= static_cast<uint8_t>(*p);
            }
            char hex[3] = {star[1], star[2], '\0'};
            uint8_t expected = static_cast<uint8_t>(strtol(hex, nullptr, 16));
            if (calc != expected) return; // bad checksum
        }

        // Work on a mutable copy so _field() can insert '\0' terminators.
        char copy[128];
        strncpy(copy, sentence, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';

        // Strip checksum suffix from copy.
        char* s = strchr(copy, '*');
        if (s) *s = '\0';

        if (strncmp(copy, "$GPRMC", 6) == 0 ||
            strncmp(copy, "$GNRMC", 6) == 0) {
            _parseRMC(copy);
        } else if (strncmp(copy, "$GPGGA", 6) == 0 ||
                   strncmp(copy, "$GNGGA", 6) == 0) {
            _parseGGA(copy);
        }
    }

    // $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,...
    void _parseRMC(char* s) {
        const char* time_f  = _field(s, 1);
        const char* status  = _field(s, 2);
        const char* lat     = _field(s, 3);
        const char* lat_dir = _field(s, 4);
        const char* lon     = _field(s, 5);
        const char* lon_dir = _field(s, 6);
        const char* speed   = _field(s, 7);
        const char* course  = _field(s, 8);

        _data.valid = (status[0] == 'A');

        if (_data.valid) {
            // Parse time
            if (strlen(time_f) >= 6) {
                char tmp[3] = {0};
                strncpy(tmp, time_f,     2); _data.hour   = atoi(tmp);
                strncpy(tmp, time_f + 2, 2); _data.minute = atoi(tmp);
                strncpy(tmp, time_f + 4, 2); _data.second = atoi(tmp);
            }
            _data.latitude    = _parseLatLon(lat, lat_dir, 2);
            _data.longitude   = _parseLatLon(lon, lon_dir, 3);
            _data.speed_kmh   = atof(speed) * 1.852f; // knots → km/h
            _data.heading_deg = atof(course);
        }
    }

    // $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,...
    void _parseGGA(char* s) {
        const char* alt  = _field(s, 9);
        const char* sats = _field(s, 7);
        _data.altitude_m  = atof(alt);
        _data.satellites  = atoi(sats);
    }
};

#endif // GPS_HANDLER_H
