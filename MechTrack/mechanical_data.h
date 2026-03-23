#ifndef MECHANICAL_DATA_H
#define MECHANICAL_DATA_H

#include <Arduino.h>
#include "config.h"

// ============================================================
// Mechanical Data
// Reads analog/digital sensors and exposes the latest values
// through a simple MechData struct.
//
// Supported channels:
//  • Engine RPM       – pulse counting on an interrupt pin
//  • Coolant temperature – NTC thermistor
//  • Oil temperature     – NTC thermistor
//  • Oil pressure        – resistive / 0-5 V sensor
//  • Throttle position   – 0-5 V potentiometer
//  • Battery voltage     – ADC via resistive divider
// ============================================================

struct MechData {
    uint16_t rpm;           // Engine RPM
    float    coolant_temp;  // Coolant temperature  (°C)
    float    oil_temp;      // Oil temperature       (°C)
    float    oil_pressure;  // Oil pressure          (bar)
    float    throttle_pct;  // Throttle position     (0–100 %)
    float    battery_v;     // Battery voltage       (V)
    bool     alert;         // True if any threshold is exceeded
};

class MechanicalData {
public:
    MechanicalData()
        : _pulse_count(0), _last_rpm_ms(0), _rpm(0)
    {
        memset(&_data, 0, sizeof(_data));
    }

    void begin() {
        pinMode(PIN_RPM_SIGNAL, INPUT_PULLUP);
        pinMode(PIN_ALERT_LED, OUTPUT);
        digitalWrite(PIN_ALERT_LED, LOW);
        // Attach interrupt – rising edge of the ignition signal.
        // The instance pointer is stored in a file-scope variable so the
        // static ISR can reach it.
        _instance = this;
        attachInterrupt(digitalPinToInterrupt(PIN_RPM_SIGNAL),
                        _rpmISR, RISING);
    }

    // Call from loop() to refresh all sensor readings.
    void update() {
        _updateRPM();
        _data.coolant_temp = _readThermistor(PIN_COOLANT_TEMP);
        _data.oil_temp     = _readThermistor(PIN_OIL_TEMP);
        _data.oil_pressure = _readOilPressure();
        _data.throttle_pct = _readThrottle();
        _data.battery_v    = _readBatteryVoltage();
        _data.rpm          = _rpm;
        _checkAlerts();
    }

    const MechData& data() const { return _data; }

    // Allow external code to trigger the RPM interrupt handler in tests.
    void triggerRpmPulse() { _pulseISR(); }

private:
    volatile uint16_t _pulse_count;
    unsigned long     _last_rpm_ms;
    uint16_t          _rpm;
    MechData          _data;

    static MechanicalData* _instance;

    // ---- ISR (must be static) ----------------------------------------
    static void _rpmISR() {
        if (_instance) _instance->_pulseISR();
    }

    void _pulseISR() {
        _pulse_count++;
    }

    // ---- RPM calculation ---------------------------------------------
    // Assumes 1 pulse per revolution.  Adjust the divisor if your
    // ignition coil fires multiple times per revolution.
    void _updateRPM() {
        unsigned long now = millis();
        unsigned long dt  = now - _last_rpm_ms;
        if (dt >= 200) { // update every 200 ms
            noInterrupts();
            uint16_t count = _pulse_count;
            _pulse_count   = 0;
            interrupts();
            // count pulses in dt ms → rev/min
            _rpm          = static_cast<uint16_t>((static_cast<unsigned long>(count) * 60000UL) / dt);
            _last_rpm_ms  = now;
        }
    }

    // ---- Thermistor --------------------------------------------------
    static float _readThermistor(uint8_t pin) {
        int raw = analogRead(pin);
        if (raw <= 0) return 0.0f;
        // Voltage divider: Vcc – THERMISTOR_SERIES_R – [ADC pin] – NTC – GND
        float r_ntc = THERMISTOR_SERIES_R *
                      (1023.0f / static_cast<float>(raw) - 1.0f);
        // Steinhart-Hart (simplified B-parameter equation)
        float t_kelvin = 1.0f /
            (1.0f / (THERMISTOR_NOMINAL_T + 273.15f) +
             (1.0f / THERMISTOR_BCOEFF) *
             log(r_ntc / THERMISTOR_NOMINAL_R));
        return t_kelvin - 273.15f;
    }

    // ---- Oil pressure ------------------------------------------------
    static float _readOilPressure() {
        float v = analogRead(PIN_OIL_PRESSURE) * (ADC_VREF / 1023.0f);
        v = constrain(v, OIL_PRESS_V_MIN, OIL_PRESS_V_MAX);
        return (v - OIL_PRESS_V_MIN) /
               (OIL_PRESS_V_MAX - OIL_PRESS_V_MIN) * OIL_PRESS_MAX_BAR;
    }

    // ---- Throttle position -------------------------------------------
    static float _readThrottle() {
        return analogRead(PIN_THROTTLE_POS) * (100.0f / 1023.0f);
    }

    // ---- Battery voltage ---------------------------------------------
    static float _readBatteryVoltage() {
        float v_adc = analogRead(PIN_BATTERY_VOLT) * (ADC_VREF / 1023.0f);
        // Undo the resistive divider to get the actual battery voltage.
        return v_adc * (BATT_R1_OHMS + BATT_R2_OHMS) / BATT_R2_OHMS;
    }

    // ---- Alert logic -------------------------------------------------
    void _checkAlerts() {
        bool alert = (_data.coolant_temp > ALERT_COOLANT_MAX_C)  ||
                     (_data.oil_temp     > ALERT_OIL_TEMP_MAX_C) ||
                     (_data.oil_pressure < ALERT_OIL_PRESS_MIN)  ||
                     (_data.battery_v    < ALERT_BATTERY_MIN_V);
        _data.alert = alert;
        digitalWrite(PIN_ALERT_LED, alert ? HIGH : LOW);
    }
};

// File-scope instance pointer used by the static ISR.
MechanicalData* MechanicalData::_instance = nullptr;

#endif // MECHANICAL_DATA_H
