# AquaShield

AquaShield is a two-device maritime-boundary warning system. The Boat device is
an ESP32 that uses simulated GPS coordinates, calculates its distance from the
Bangladesh maritime boundary, activates local warnings, and transmits its
status. The Coast Guard device is an ESP8266 that receives the status and
shows it on an OLED display.

The boat route is simulated for the presentation. The boundary itself is real
data converted from the supplied lat_lon_maritime_bgd.csv file.

## Project structure

~~~
AquaShield/
├── Boat/                 ESP32 firmware
│   ├── boat.ino          Main Boat sketch
│   ├── config.h          ESP32 pins, radio settings, map origin, thresholds
│   ├── segments.h        Generated real maritime-boundary line segments
│   ├── geo.h             Latitude/longitude to local-metre conversion
│   ├── distance.h        Shortest point-to-segment distance calculation
│   ├── warning.h         LED and buzzer warning behaviour
│   ├── radio.h           CC1101 packet transmitter
│   └── simulation.h      Dummy boat GPS route
├── CoastGuard/           ESP8266 firmware
│   ├── receiver.ino      Main Coast Guard sketch
│   ├── config.h          ESP8266, OLED, buzzer, and radio settings
│   ├── radio.h           CC1101 packet receiver
│   └── display.h         OLED display functions
└── README.md             This documentation
~~~

The projects are independent. Upload Boat/boat.ino to the ESP32 and
CoastGuard/receiver.ino to the ESP8266. Do not upload either sketch to the
other board.

For a browser-based simulation and a concise guide showing exactly which sketch
to upload to each physical board, see [Wokwi/README.md](Wokwi/README.md).

The Wokwi simulation has its own short six-point presentation route. The
physical Boat firmware described below keeps its separate 16-point route in
`Boat/simulation.h`.

## Required hardware

### Boat device

- ESP32 DevKit V1
- nRF24L01+ module (2.4 GHz) with 10–100µF capacitor across VCC/GND
- One green LED and 220 ohm resistor
- One red LED and 220 ohm resistor
- Active buzzer
- Jumper wires and common ground

### Coast Guard device

- NodeMCU ESP8266 (ESP-12E)
- nRF24L01+ module (2.4 GHz) with 10–100µF capacitor across VCC/GND
- 16x2 I2C LCD (LCD1602 with PCF8574 backpack, address 0x27 or 0x3F)
- Active buzzer
- Jumper wires and common ground

## Install Arduino support and libraries

Use Arduino IDE 2.x. Install the board packages before compiling the sketches:

1. Open **File > Preferences**.
2. Add these URLs to **Additional Boards Manager URLs**:

   ~~~
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ~~~

3. Open **Tools > Board > Boards Manager**.
4. Install **esp32 by Espressif Systems**.
5. Install **esp8266 by ESP8266 Community**.
6. Open **Tools > Manage Libraries** and install these libraries:

   | Library | Author | Why it is required |
   |---|---|---|
   | **RF24** | TMRh20 | Configures nRF24L01+ and handles 2.4 GHz packet transmission/reception. |
   | **LiquidCrystal I2C** | Frank de Brabander | Controls the Coast Guard 16x2 I2C LCD. |

## Wiring

All nRF24L01+ modules must use **3.3 V only**. Never connect nRF24L01+ VCC to 5 V. Connect the GND of every module, LED, buzzer, and development board together.

### ESP32 Boat wiring

| Part | ESP32 Pin | Notes |
|---|---:|---|
| nRF24L01+ VCC | 3V3 | **3.3 V only** (place 10–100µF capacitor across VCC/GND) |
| nRF24L01+ GND | GND | Common ground |
| nRF24L01+ SCK | GPIO 18 | VSPI Clock |
| nRF24L01+ MISO | GPIO 19 | VSPI MISO |
| nRF24L01+ MOSI | GPIO 23 | VSPI MOSI |
| nRF24L01+ CSN | GPIO 5 | Chip Select |
| nRF24L01+ CE | GPIO 4 | Chip Enable |
| Green LED anode | GPIO 26 through 220Ω resistor | LED cathode to GND |
| Red LED anode | GPIO 27 through 220Ω resistor | LED cathode to GND |
| Active buzzer input | GPIO 25 | Active buzzer (-) to GND |

### ESP8266 Coast Guard wiring

| Part | NodeMCU Label | GPIO | Notes |
|---|---|---:|---|
| 16x2 LCD VCC | VIN | - | **5 V required** for readable display contrast |
| 16x2 LCD GND | GND | - | Common ground |
| 16x2 LCD SDA | D2 | 4 | I2C Data |
| 16x2 LCD SCL | D1 | 5 | I2C Clock |
| nRF24L01+ VCC | 3V3 | - | **3.3 V only** (place 10–100µF capacitor across VCC/GND) |
| nRF24L01+ GND | GND | - | Common ground |
| nRF24L01+ SCK | D5 | 14 | Hardware HSPI Clock |
| nRF24L01+ MISO | D6 | 12 | Hardware HSPI MISO |
| nRF24L01+ MOSI | D7 | 13 | Hardware HSPI MOSI |
| nRF24L01+ CSN | D8 | 15 | Chip Select (pulled LOW at boot) |
| nRF24L01+ CE | D4 | 2 | Chip Enable |
| Active buzzer input | D0 | 16 | GPIO 16 is boot-safe (won't cause boot failure) |
| Active buzzer (-) | GND | - | Common ground |

## Upload each firmware

### Upload the Boat firmware to the ESP32

1. Connect the ESP32 by USB.
2. In Arduino IDE, open Boat/boat.ino.
3. Select **Tools > Board > esp32 > ESP32 Dev Module**.
4. Select the ESP32 serial port in **Tools > Port**.
5. Click **Upload**.
6. Open Serial Monitor at **115200 baud** to see position, distance, and
   warning messages.

### Upload the Coast Guard firmware to the ESP8266

1. Connect the NodeMCU by USB.
2. In Arduino IDE, open CoastGuard/receiver.ino.
3. Select **Tools > Board > ESP8266 Boards > NodeMCU 1.0 (ESP-12E Module)**.
4. Select the NodeMCU serial port in **Tools > Port**.
5. Click **Upload**.
6. Open Serial Monitor at **115200 baud** to monitor received boat packets.

Upload the Coast Guard device first, then the Boat device. The receiver shows
“Waiting boat...” on the 16x2 LCD until it receives the first valid radio packet.

## What each board does

### ESP32 Boat

At boot, simulation.h provides the first dummy GPS point. Every five seconds it
advances to the next point. The Boat sketch converts that point to local X/Y
metres, finds the closest real boundary segment, chooses a warning level,
updates the LEDs and buzzer, and transmits a packet via nRF24L01+ once per second. This
continues at every risk level, including DANGER.

| Distance from boundary | Green LED | Red LED | Buzzer |
|---|---|---|---|
| More than 2000 m | On | Off | Off |
| 2000 m or less, more than 1000 m | Off | Slow blink | Slow beep |
| 1000 m or less | Off | On | Continuous / fast alarm |

### ESP8266 Coast Guard

The ESP8266 maintains the nRF24L01+ in listening mode. When a packet arrives,
it parses the coordinates, distance, and status, sound the active buzzer
if in DANGER, and rotates through 3 clean informational screens on the 16x2 LCD:
- Screen 1: Real-time Latitude & Longitude
- Screen 2: Boundary Distance & Status (SAFE / WARNING / DANGER)
- Screen 3: Boat ID & Maritime Territory (BANGLADESH / OTHER SEA)
Both boards share identical nRF24L01+ settings: Channel 108 (2508 MHz), 250 kbps,
pipe address `0xF0F0F0F0E1LL`, and the same packet identifier (`0x41515348`). Those settings
are defined in each sketch's `config.h`.

## Real-boundary CSV conversion

The supplied CSV contains 184 ordered boundary vertices:

~~~
X,Y,vertex_index,modified_vertex_index
92.368667,20.704389,3618,1
...
~~~

For this data, X is longitude and Y is latitude. segments.h was generated from
every consecutive pair of rows, so it contains **183 line segments**. The first
CSV point, latitude 20.704389 and longitude 92.368667, is used as the local
map origin in Boat/config.h. This keeps the generated X/Y values small enough
for accurate float calculations on the ESP32.

Do not edit Boat/segments.h by hand. If the approved boundary CSV changes,
regenerate all segments in the same row order, update the two map-origin values
to the first row, and keep NUMBER_OF_BOUNDARY_SEGMENTS equal to the number of
CSV points minus one.

## Boundary-distance logic

GPS coordinates are in degrees, but warning distances must be in metres. For
each boat location, geo.h uses a local equirectangular map approximation:

~~~
x = EarthRadius × cos(originLatitude) × (longitude - originLongitude)
y = EarthRadius × (latitude - originLatitude)
~~~

The angle differences are converted from degrees to radians first. The map is
centred on the first real boundary point, so it is appropriate for this regional
distance warning. For legal navigation or enforcement, use an approved GIS and
a geodesic calculation; this university project is a warning aid, not a
certified navigation system.

distance.h checks every boundary segment. For one segment it projects the boat
point onto the infinite line, clamps that projection to the segment ends, then
calculates the straight-line distance to the nearest point. It keeps the
smallest result across all 183 segments. This is why the warning is based on
the nearest part of the full real CSV boundary, not just one selected point.

## Dummy Boat route used in the presentation

The 16 GPS values in Boat/simulation.h are deliberately simulated. They model
a boat approaching the real imported boundary, entering the danger zone, then
moving back out. Each point's risk is calculated at runtime from its nearest
boundary distance; no warning level is stored in the route itself.

| Route point | Approximate nearest-boundary distance | Result |
|---|---:|---|
| 20.670400, 92.376200 | 2245 m | SAFE |
| 20.673050, 92.373550 | 2236 m | SAFE |
| 20.675700, 92.370900 | 2227 m | SAFE |
| 20.678350, 92.368250 | 2029 m | SAFE |
| 20.681000, 92.365600 | 1625 m | WARNING |
| 20.682700, 92.364067 | 1379 m | WARNING |
| 20.684400, 92.362533 | 1132 m | WARNING |
| 20.686100, 92.361000 | 886 m | DANGER |
| 20.687167, 92.360000 | 728 m | DANGER |
| 20.688233, 92.359000 | 571 m | DANGER |
| 20.689300, 92.358000 | 413 m | DANGER |
| 20.688233, 92.359000 | 571 m | DANGER |
| 20.686100, 92.361000 | 886 m | DANGER |
| 20.682700, 92.364067 | 1379 m | WARNING |
| 20.678350, 92.368250 | 2029 m | SAFE |
| 20.673050, 92.373550 | 2236 m | SAFE |

The route repeats automatically after the sixteenth point. No code editing is
needed during a presentation.

### Confirming danger-zone radio delivery

With both boards powered and their Serial Monitors set to **115200 baud**, wait
for a DANGER route point (about 35 seconds after boot). The Boat prints
`DANGER packet transmitted` once per second after each successful CC1101
transmission. The Coast Guard prints `Boat 01 distance: ... m status: DANGER`
for every valid packet it receives and turns its buzzer on. Seeing both messages
confirms that DANGER data is transmitted and received. If the Boat reports a
transmission but the Coast Guard prints nothing, use the receiver troubleshooting
steps below to check the radio wiring, settings, and antennas.

To later use a real GPS module, replace only Boat/simulation.h while keeping
the same GPSPoint structure and these two functions:

~~~
void startLocationSource();
bool readLocation(GPSPoint *currentGpsPoint);
~~~

readLocation must put a newly available GPS position into currentGpsPoint and
return true; it should return false when there is no new valid position. The
remaining Boat code does not need to change.

## Radio packet contents

The Boat sends a fixed binary packet containing:

- an AquaShield packet identifier used to reject unrelated radio data;
- Boat ID (01 by default);
- latitude and longitude as floating-point values;
- calculated nearest-boundary distance in metres; and
- warning level: SAFE, WARNING, or DANGER.

The Boat ID can be changed in Boat/config.h. If multiple boats are added, give
every Boat firmware copy a different ID while retaining identical radio settings
and packet structure.

## Troubleshooting

- **CC1101 transmitter/receiver not found**: check 3.3 V, common ground, CSN,
  and all four SPI/GDO0 connections. Ensure the module is a CC1101, not a
  similarly shaped 2.4 GHz module.
- **OLED is blank**: confirm 3.3 V, GND, SDA D2, and SCL D0. If the module uses
  address 0x3D, change OLED_I2C_ADDRESS in CoastGuard/config.h.
- **Receiver never gets data**: both radios must use 433 MHz modules and
  matching antennas. Keep them a few metres apart for the first test.
- **No LED or buzzer output**: verify the active buzzer is active-high and LED
  polarity is correct. Use a transistor driver if the buzzer needs more current
  than a GPIO can provide.
- **Distance seems wrong after changing CSV data**: regenerate every segment and
  update the map origin from the first CSV row. Do not mix one CSV with an old
  segments.h file.

## Safety and operating note

This firmware is suitable for a university demonstration. It must not be the
only navigation or boundary-enforcement system on a real boat. Verify local RF
rules for 433 MHz, use approved marine navigation equipment, and validate the
source boundary data before any real-world operation.

---

## NodeMCU ESP8266 + nRF24L01+ + 16x2 I2C LCD Version (`NodeMCU_Device/`)

For setups using **NodeMCU ESP8266 (ESP-12E)**, **nRF24L01+ (2.4 GHz)**, **16x2 I2C LCD**, and an **Active Buzzer**, use the sketch in `NodeMCU_Device/`.

### Architecture

1. **Simulated GPS Transmitter (`GPS_Transmitter/`)**:
   - Runs on another microcontroller (Arduino Nano/Uno, ESP32, or ESP8266) equipped with an nRF24L01+.
   - Transmits the 16-point simulated GPS coordinates every 3 seconds over 2.4 GHz (RF Channel 108).
2. **NodeMCU Warning & Display Unit (`NodeMCU_Device/`)**:
   - Receives the simulated GPS coordinates via nRF24L01+.
   - Computes distance to the 183 Bangladesh maritime boundary segments locally.
   - Triggers the active buzzer (Silent in SAFE, intermittent in WARNING, rapid pulsing in DANGER).
   - Displays real-time coordinates, boundary distance, and warning status across 3 rotating screens on the 16x2 I2C LCD.

### NodeMCU Wiring

| Component | Component Pin | NodeMCU Pin | GPIO | Notes |
|---|---|---|---:|---|
| **16x2 I2C LCD** | VCC | VIN (5V) | - | 5V required for readable contrast |
| | GND | GND | - | Common Ground |
| | SDA | D2 | GPIO 4 | I2C Data |
| | SCL | D1 | GPIO 5 | I2C Clock |
| **nRF24L01+** | VCC | 3V3 (3.3V) | - | **3.3V ONLY!** Add 10-100µF capacitor across VCC/GND |
| | GND | GND | - | Common Ground |
| | SCK | D5 | GPIO 14 | Hardware HSPI Clock |
| | MISO | D6 | GPIO 12 | Hardware HSPI MISO |
| | MOSI | D7 | GPIO 13 | Hardware HSPI MOSI |
| | CSN | D8 | GPIO 15 | Chip Select (Pulled LOW at boot) |
| | CE | D4 | GPIO 2 | Chip Enable |
| **Active Buzzer** | (+) Signal | D0 | GPIO 16 | GPIO 16 is boot-safe (won't hang ESP8266 boot) |
| | (-) GND | GND | - | Common Ground |

### Required Arduino Libraries

Install these via **Tools > Manage Libraries...** in Arduino IDE:
1. `RF24` by TMRh20
2. `LiquidCrystal I2C` by Frank de Brabander

