
MeckesMek GPS Module – System Description
Contents
1. System Overview	2
2. Hardware	2
2.1 Core	2
2.2 GNSS Receiver	2
2.3 Display	2
2.4 Battery	2
3. Firmware Architecture	3
3.1 Modules	3
3.2 Data Flow	3
4. Interfaces & Transports	3
4.1 Wi‑Fi (NMEA over TCP)	3
4.2 Bluetooth Low Energy (RaceChrono compatible)	3
4.3 USB serial	3
5. Web Interface (Port 80)	3
5.1 Status Page	3
5.2 Display Settings	4
5.3 Wi‑Fi Settings	4
5.4 BLE Name	4
5.5 Firmware Update (OTA)	4
6. OLED Indicators & Boot Messages	4
7. Setup & Connection Guides	4
7.1 RaceChrono via Wi‑Fi	4
7.2 RaceChrono via BLE	5
7.3 USB Passthrough for GNSS Configuration	5
7.4 GPS Configuration via Web UI	5
8. Operational Notes	5
8.1 Concurrent Wi‑Fi + BLE	5
8.2 NMEA Sentences & Rates	5
8.3 Performance & MTU	5
9. Troubleshooting	6
10. Known Limitations / To‑Do	6
11. Change Log	6

1. System Overview
Standalone GNSS telemetry module based on ESP32‑S3 and u‑blox M10Q. Streams NMEA 0183 over Wi‑Fi (TCP port 10110) and/or over Bluetooth Low Energy (RaceChrono‑compatible custom service). Includes on‑device Web UI for settings and OTA, and an OLED for live status. USB serial passthrough enables direct GNSS configuration.
•	Key capabilities:
•	Wi‑Fi Access Point with static IP 192.168.50.1; NMEA over TCP on port 10110.
•	BLE telemetry compatible with RaceChrono (20‑byte notifications).
•	USB serial passthrough for GNSS baud/rate/UBX configuration.
•	OLED status (battery %, fix, PDOP, update rate), shows SSID/PASS at boot.
•	Web UI (port 80) for status, settings, BLE name, and firmware update (OTA).
2. Hardware
2.1 Core
ESP32‑S3 module (dual‑core, Wi‑Fi + BLE). Power via USB‑C.
2.2 GNSS Receiver
u‑blox M10Q, default baud 230400 and with a Position Update-Rate of 15hz (. Produces standard NMEA sentences (GGA, GSA, RMC, VTG, etc.).
2.3 Display
128×32 OLED shows battery status, fix status, PDOP, update rate, Wi‑Fi icon, and “BT” when BLE is connected to RaceChrono.
2.4 Battery
Lipo battery with safety circuit. 3.7v 2000mah and is charged via the USB-C port.

3. Firmware Architecture
3.1 Modules
- gps_task.*: Reads UART, assembles NMEA lines, parses metrics (GGA/GSA) and populates snapshot for BLE.
- transport_wifi.*: SoftAP setup + TCP server (10110) + client handling.
- transport_ble.*: BLE service/characteristics for RaceChrono; sends 20‑byte packets.
- webui.*: Web server (80) – status JSON, settings, BLE name, OTA endpoint.
- display.*: OLED boot screen, periodic status (2 Hz), Wi‑Fi icon & BT tag.
- state.*: Preferences (NVS) backed configuration and live metrics.
3.2 Data Flow
UART → gps_task: builds NMEA lines →
• For Wi‑Fi: full NMEA lines are pushed to TCP client if connected.
• For BLE: parsed snapshot (time, lat/lon, speed, course, satellites, HDOP/VDOP) is packed and notified.
• For USB: when no Wi‑Fi TCP client is connected, raw NMEA is mirrored to USB; host input is forwarded to GPS UART.
4. Interfaces & Transports
4.1 Wi‑Fi (NMEA over TCP)
SoftAP SSID and password are configurable in the Web UI. Static IP 192.168.50.1. RaceChrono (or any NMEA client) connects to TCP port 10110. If no TCP client is connected, NMEA is still produced but not sent over Wi‑Fi (USB passthrough remains active).
4.2 Bluetooth Low Energy (RaceChrono compatible)
BLE advertising name is configurable in the Web UI. Implements the RaceChrono GNSS service and characteristics; supports high‑rate updates. When connected, the OLED shows “BT” next to the battery line.
4.3 USB serial
Open a serial terminal on the MCU’s USB port. When no Wi‑Fi TCP client is active, the device mirrors NMEA to USB and forwards terminal input to the GPS UART. Useful for changing GNSS output rate or issuing UBX configuration commands.

5. Web Interface (Port 80)
5.1 Status Page
Live Battery %, Fix (No/2D/3D), PDOP (m), and GGA rate (Hz).
5.2 Display Settings
Startup message and rotation (180°).
5.3 Wi‑Fi Settings
SSID and password; changes are stored in NVS. A reboot applies the new settings.
(NOTE: Remember to connect to the new Wifi name and password)
5.4 BLE Name
Change the BLE advertising name. Requires reboot to take effect.
(NOTE: Remember to update the connection in RaceChrono, See section 7.2)
5.5 Firmware Update (OTA)
Upload a compiled .bin via the Firmware Update section. When you have selected the file and pressed “Upload” The screen will freeze for around 10-15 seconds before you get a confirmation that a new firmware has been installed. Reboot the GPS via the on/off switch.
 
(NOTE: If you do it on a PC. The “Choose File” button might not work. Just drag and drop the bin file at the “Choose File” button.) 
6. OLED Indicators & Boot Messages
- Boot: shows startup message and firmware version, then SSID and PASS for a few seconds to aid onboarding.
- Status: Battery %, Wi‑Fi icon if a TCP client is connected, “BT” when BLE is connected.
- GPS: Fix status (No/2D/3D), HDOP (m), and update rate (Hz).









7. Setup & Connection Guides
7.1 RaceChrono via Wi‑Fi
1) On your phone, connect to the device’s Wi‑Fi.

2) In RaceChrono → Settings → Other Devices → Add other device → RaceChrono DIY → TCP/IP.

3) Check: NMEA 0183
     Check: Connect Wi-Fi Automatically
      Type in:
                 * Wifi name (SSID): Your-Wifi-Name			Default: MeckesMek_GPS
                 * Password: Your password 				Default: 12345678
                 * IP-address: 192.168.50.1
                 * Port number : 10110
(NOTE: If you forgot Wifi name and password. Restart the GPS device and you will see on the Oled display your WIFI name and password) 

4) Save and start a session; the OLED will show the Wi‑Fi icon when the client connects.

7.2 RaceChrono via BLE
1) In RaceChrono → Settings → Other Devices → Add other device → RaceChrono DIY → Bluetooth LE → Search for devices in range.

2) Select the device by its BLE name.

3) Save and start a session; the OLED shows “BT” when connected.

7.3 USB Passthrough for GNSS Configuration
1) Connect USB and open a serial terminal at the module baud (e.g., 230400).

2) When no Wi‑Fi TCP client is connected, NMEA is mirrored to USB.

3) Type commands to forward them to the GPS UART (e.g., UBX / NMEA config). Via the Ublox U-Center Software. (WARNING: If low experience on GPS configuration leaves it as it)


7.4 GPS Configuration via Web UI
1) On your phone, connect to the device’s Wi‑Fi.

2) Go to a web browser and type in the address: 192.168.50.1 or 192.168.50.1;80

3) Here you can change SSID (Wifi name, Wifi Password, Flip the display, Bluetooth name, Upload new firmware, etc. (See section 5)

4) After a change in the webUI and pressed save. Power Cycle the GPS Module (on/off) for the changes to take effect. 
8. Operational Notes
8.1 Concurrent Wi‑Fi + BLE
Both transports can be active simultaneously. BLE takes priority for RaceChrono, while the Web UI remains reachable over Wi‑Fi.
8.2 NMEA Sentences & Rates
Device forwards full NMEA over Wi‑Fi (GGA, RMC, VTG, GSA…). BLE uses compact binary packets.
8.3 Performance & MTU
BLE MTU set to 517; preferred 2M PHY. Typical stable rate ~15 Hz.
9. Troubleshooting
- Can’t see Web UI: Connect to AP SSID shown at boot; browse to http://192.168.50.1/.
- RaceChrono via Wi‑Fi doesn’t connect: Verify TCP 10110 and AP password.
- BLE name change didn’t apply: Reboot after saving the new BLE name.
- No NMEA over USB: Ensure no Wi‑Fi TCP client is connected (USB mirroring is paused while a TCP client is active).
