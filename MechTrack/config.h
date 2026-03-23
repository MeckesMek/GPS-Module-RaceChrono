#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// MechTrack Configuration
// Edit these settings to match your hardware setup.
// ============================================================

// --- GPS ---
// Baud rate for the GPS module serial port
#define GPS_BAUD_RATE       9600

// --- Bluetooth ---
// Baud rate for the Bluetooth module (HC-05 / HC-06)
#define BT_BAUD_RATE        115200

// Name advertised over Bluetooth (max 20 characters)
#define BT_DEVICE_NAME      "MechTrack"

// --- RaceChrono channel IDs ---
// These IDs identify the custom channels inside RaceChrono.
// They must match the channel IDs configured in the RaceChrono app.
#define RC_CHANNEL_RPM        1
#define RC_CHANNEL_COOLANT    2
#define RC_CHANNEL_OIL_TEMP   3
#define RC_CHANNEL_OIL_PRESS  4
#define RC_CHANNEL_THROTTLE   5
#define RC_CHANNEL_BATTERY    6

// --- Analog sensor pins ---
#define PIN_RPM_SIGNAL      2   // Digital pin – interrupt-capable
#define PIN_COOLANT_TEMP    A0  // Analog pin
#define PIN_OIL_TEMP        A1  // Analog pin
#define PIN_OIL_PRESSURE    A2  // Analog pin
#define PIN_THROTTLE_POS    A3  // Analog pin
#define PIN_BATTERY_VOLT    A4  // Analog pin (via voltage divider)

// --- Alert thresholds ---
#define ALERT_COOLANT_MAX_C   105   // °C  – warning LED on above this value
#define ALERT_OIL_TEMP_MAX_C  130   // °C
#define ALERT_OIL_PRESS_MIN   1.0f  // bar – warning LED on below this value
#define ALERT_BATTERY_MIN_V   11.5f // V   – warning LED on below this value

// --- Alert output pin ---
#define PIN_ALERT_LED       13  // Built-in LED used as alert indicator

// --- Thermistor calibration (Steinhart-Hart coefficients) ---
// Default values suit a generic NTC 10 kΩ thermistor with a 10 kΩ pull-up.
#define THERMISTOR_SERIES_R   10000.0f  // Pull-up resistor value in ohms
#define THERMISTOR_NOMINAL_R  10000.0f  // Resistance at nominal temperature
#define THERMISTOR_NOMINAL_T  25.0f     // Nominal temperature in °C
#define THERMISTOR_BCOEFF     3950.0f   // Beta coefficient

// --- Oil pressure sensor calibration ---
// Maps 0–5 V output to bar. Adjust for your specific sensor.
#define OIL_PRESS_V_MIN   0.5f   // Volts at 0 bar
#define OIL_PRESS_V_MAX   4.5f   // Volts at OIL_PRESS_MAX_BAR
#define OIL_PRESS_MAX_BAR 10.0f  // Full-scale pressure in bar

// --- Battery voltage divider ---
// R1 is the top resistor (between battery+ and the ADC pin).
// R2 is the bottom resistor (between the ADC pin and GND).
// Typical values: R1 = 30 kΩ, R2 = 10 kΩ  → 0-16 V range on a 5 V ADC.
#define BATT_R1_OHMS      30000.0f
#define BATT_R2_OHMS      10000.0f

// --- Data transmission interval (milliseconds) ---
#define TRANSMIT_INTERVAL_MS  100   // Send data to RaceChrono every 100 ms

// --- ADC reference voltage ---
#define ADC_VREF              5.0f  // Volts

#endif // CONFIG_H
