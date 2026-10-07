# AquaShield: complete project explanation

This document explains AquaShield in simple language, but it is also precise
enough to use when a teacher asks technical questions. Read the **short answer
for a teacher** first. The later sections explain every project file, every
major calculation, the real-hardware version, and the Wokwi presentation.

## 1. Short answer for a teacher

**AquaShield is a two-device maritime-boundary warning prototype.** A Boat
device has a GPS location. It calculates its shortest distance from a stored
Bangladesh maritime-boundary line. When the boat is close to that line, it
activates visual/audio warnings and sends the boat position, distance, and
warning level to a Coast Guard device. The Coast Guard device displays the
same information and sounds an alarm during danger.

For the presentation, the GPS position is deliberately simulated. The Wokwi
version replaces the physical radio with MQTT over simulated Wi-Fi, because
Wokwi does not model the exact real radio hardware used by the project.

## 2. The important honest facts

- The project does **not** currently read a live GPS receiver. It reads
  prepared test coordinates from a route in the code.
- The original boundary came from a CSV file named `lat_lon_maritime_bgd.csv`.
  That CSV and the program that generated the header are **not included in the
  current repository**. The generated result is included in `segments.h`.
- The project calculates **nearest distance to the boundary**. It does not
  determine legally whether a boat is on the Bangladesh side or the other
  side of the line. A real legal-navigation system needs approved data,
  geodesic/GIS methods, live GPS validation, radio reliability testing, and
  safety approval.
- Wokwi proves the software flow: route → calculation → status message →
  receiver → display/alarm. It does not prove real CC1101 radio range,
  antenna performance, power wiring, or ESP8266 behaviour.

These are not hidden weaknesses. They are correct limits to explain clearly
during a university demonstration.

## 3. Basic words used in this project

| Word | Simple meaning |
|---|---|
| **ESP32** | A small programmable microcontroller. The Boat uses one; Wokwi uses one for both simulated devices. |
| **ESP8266 / NodeMCU** | Another small microcontroller. The real Coast Guard device uses it. |
| **GPS coordinate** | A location written as latitude and longitude, for example `20.686100, 92.361000`. |
| **Latitude** | North/south position on Earth. |
| **Longitude** | East/west position on Earth. |
| **Boundary** | The imported maritime line that the boat must not approach too closely. |
| **Segment** | One small straight piece of the long boundary line. |
| **CC1101** | The real 433 MHz radio module used between the physical devices. |
| **MQTT** | A publish/subscribe messaging method. Wokwi uses it as a virtual radio link. |
| **OLED** | A small display used by the Coast Guard device. |
| **Firmware** | The program compiled and uploaded to a microcontroller. |
| **Header file (`.h`)** | A C++ file containing reusable definitions and functions that another source file includes. |

## 4. System picture

### Physical prototype

```text
Prepared GPS point (currently simulated)
             |
             v
      ESP32 Boat firmware
  convert to metres; find nearest
  boundary segment; choose risk
       |             |
       |             +--> green/red LED and buzzer
       v
  CC1101 radio, 433 MHz
       |
       v
 NodeMCU ESP8266 Coast Guard
       |             |
       v             +--> danger buzzer
 OLED shows ID, location, distance, status
```

### Wokwi presentation version

```text
Six prepared GPS points
             |
             v
      Simulated ESP32 Boat
  same boundary-distance calculation
       |
       v
 MQTT topic on broker.emqx.io
       |
       v
 Simulated ESP32 Coast Guard stand-in
       |
       +--> OLED, buzzer, Serial Monitor
```

The two diagrams are similar on purpose, but they are not the same transport:
the physical prototype uses CC1101 radio; Wokwi uses MQTT.

## 5. Repository map and every file

```text
AquaShield/
├── README.md
├── setup.md
├── description.md
├── AquaShield_July 27, 2026.rar
├── Boat/                         physical Boat firmware for an ESP32
├── CoastGuard/                   physical Coast Guard firmware for an ESP8266
└── Wokwi/                        two Wokwi/PlatformIO simulation projects
```

### Top-level files

| File | What it is and why it exists |
|---|---|
| `README.md` | Main project manual: hardware list, wiring, Arduino IDE libraries, physical upload steps, boundary method, and the physical 16-point Boat route. |
| `setup.md` | Fresh Windows-laptop setup guide: WSL2, VS Code, Wokwi, PlatformIO, commands, and presentation steps. |
| `description.md` | This complete explanation document. |
| `AquaShield_July 27, 2026.rar` | A compressed archive copy of the project. It is not used by any program while building or simulating. Extract it only if you need a backup copy. |

### `Boat/`: physical ESP32 Boat firmware

| File | Detailed job |
|---|---|
| `boat.ino` | The main program. It starts hardware, obtains a location, calculates distance/risk, updates Boat warnings, and transmits data every second. Arduino runs `setup()` once and then repeats `loop()` forever. |
| `config.h` | One place for values that the Boat uses: ID `01`, map origin, pins, radio settings, timings, and warning distances. |
| `simulation.h` | The current Boat location source. It defines 16 fake GPS coordinates and returns one every five seconds. Replace this source with real GPS-reading code for a real deployment. |
| `segments.h` | Defines `Segment` and stores 183 generated boundary segments in local metres. It is input data, not logic to edit manually. |
| `geo.h` | Converts latitude/longitude to local X/Y metres so the calculation can use metres rather than degrees. |
| `distance.h` | Finds the shortest distance from the Boat point to one segment, then to all 183 boundary segments. |
| `warning.h` | Changes the Boat green LED, red LED, and buzzer for SAFE, WARNING, and DANGER. |
| `radio.h` | Sets up the Boat CC1101 transmitter and packs Boat data into a `RadioPacket` for transmission. |

### `CoastGuard/`: physical ESP8266 Coast Guard firmware

| File | Detailed job |
|---|---|
| `receiver.ino` | Main Coast Guard program. It starts the display and radio, receives valid packets, refreshes the display, and controls the danger buzzer. |
| `config.h` | ESP8266/NodeMCU pins, OLED settings, and radio settings. The radio values must match Boat settings. |
| `radio.h` | Defines the receiver's matching `RadioPacket`, starts CC1101 receive mode, detects arrival, validates a packet, and returns it to `receiver.ino`. |
| `display.h` | Starts the SSD1306 OLED and draws Boat ID, latitude, longitude, distance, status, and `RF link active`. |

### `Wokwi/`: presentation simulation files

| File | Detailed job |
|---|---|
| `Wokwi/README.md` | Wokwi-specific guide: two-project limitation, MQTT replacement, web/VS Code run steps, and the six-point presentation route. |
| `Wokwi/Boat/sketch.ino` | Complete simulated Boat program. It uses the same distance idea as physical Boat but sends a text MQTT message instead of a real radio packet. |
| `Wokwi/Boat/segments.h` | Copy of the generated 183-segment boundary data used by the simulated Boat. It must stay consistent with the Boat calculation. |
| `Wokwi/Boat/diagram.json` | Wokwi circuit drawing: ESP32, green LED/resistor, red LED/resistor, and buzzer, plus their wire connections. |
| `Wokwi/Boat/platformio.ini` | PlatformIO build instructions: ESP32 platform/board, Arduino framework, PubSubClient dependency, and post-build copy script. |
| `Wokwi/Boat/libraries.txt` | The library name used when importing this project in the Wokwi website editor: `PubSubClient`. |
| `Wokwi/Boat/wokwi.toml` | Tells Wokwi to load `firmware.bin` and `firmware.elf` from this folder. |
| `Wokwi/Boat/copy_wokwi_firmware.py` | PlatformIO post-build helper. It copies the newly built binary and debug ELF from `.pio/build/...` into this Wokwi folder. |
| `Wokwi/Boat/compile_commands.json` | Generated C/C++ IntelliSense/compile database. VS Code can use it to understand include paths. It is not firmware source and is not needed by the microcontroller at runtime. |
| `Wokwi/Boat/firmware.bin` | Compiled machine-code binary loaded by Wokwi. It is regenerated by `pio run`; humans do not edit it. |
| `Wokwi/Boat/firmware.elf` | Compiled ELF/debug file loaded by Wokwi. It is regenerated by `pio run`; humans do not edit it. |
| `Wokwi/CoastGuard/sketch.ino` | Complete simulated Coast Guard program. It receives/parses MQTT messages, updates the OLED, and turns on the buzzer in DANGER. |
| `Wokwi/CoastGuard/diagram.json` | Wokwi circuit drawing: ESP32 stand-in, SSD1306 OLED, buzzer, and their wires. |
| `Wokwi/CoastGuard/platformio.ini` | PlatformIO configuration for ESP32 plus PubSubClient, Adafruit GFX, and Adafruit SSD1306 dependencies. |
| `Wokwi/CoastGuard/libraries.txt` | Library names used for website import: PubSubClient, Adafruit GFX Library, and Adafruit SSD1306. |
| `Wokwi/CoastGuard/wokwi.toml` | Points Wokwi to this project's generated firmware files. |
| `Wokwi/CoastGuard/copy_wokwi_firmware.py` | Copies the fresh binary/ELF after each PlatformIO build. |
| `Wokwi/CoastGuard/compile_commands.json` | Generated IntelliSense/compile database; not source and not runtime firmware. |
| `Wokwi/CoastGuard/firmware.bin` | Generated machine-code firmware for the Wokwi Coast Guard stand-in. |
| `Wokwi/CoastGuard/firmware.elf` | Generated debug/ELF file for the Wokwi Coast Guard stand-in. |

## 6. How the boundary is represented

### 6.1 From CSV points to segments

The original CSV was described as having 184 ordered rows like this:

```text
X,Y,vertex_index,modified_vertex_index
92.368667,20.704389,3618,1
...
```

In that data, `X` is **longitude** and `Y` is **latitude**. A computer joins
each point to the next point:

```text
point 1 ---------------- point 2   = segment 1
point 2 ---------------- point 3   = segment 2
point 3 ---------------- point 4   = segment 3
```

So 184 points create 183 connections, which are the 183 `boundarySegments` in
`Boat/segments.h` and `Wokwi/Boat/segments.h`.

A single segment has four numbers:

```cpp
struct Segment {
  float x1;  // first end's horizontal local-metre coordinate
  float y1;  // first end's vertical local-metre coordinate
  float x2;  // second end's horizontal local-metre coordinate
  float y2;  // second end's vertical local-metre coordinate
};
```

For example, the first stored segment is:

```cpp
{0.000, 0.000, -2837.284, -2804.558}
```

It begins at the chosen map origin `(0, 0)` and ends approximately 2.84 km west
and 2.80 km south of it. It is not four GPS degrees; it is four **metre-based
local coordinates**.

### 6.2 Why latitude/longitude becomes metres

GPS uses degrees, but a warning threshold such as 1000 metres must be compared
with metres. Therefore the project makes a local flat map centred at:

```text
latitude  = 20.704389
longitude = 92.368667
```

It uses an equirectangular approximation with Earth radius `6,371,000 m`:

```text
x = EarthRadius × cos(originLatitude) × (longitude − originLongitude)
y = EarthRadius × (latitude − originLatitude)
```

All angles are changed from degrees to radians before this formula is used.
`x` means east/west metres; `y` means north/south metres. The same origin and
formula are used for the boat and the boundary. That is why their distance can
be calculated correctly in metres for this regional student project.

### 6.3 How one Boat point is compared with one segment

Imagine a boat point `P` and a boundary segment from `A` to `B`.

1. The code finds the direction of the segment: `B − A`.
2. It projects `P` onto the infinite line through `A` and `B`.
3. A projection could fall beyond the actual ends, so the code clamps it to
   the range `0` through `1`.
4. The clamped position is the nearest real point on that finite segment.
5. It uses Pythagoras (`sqrt(dx² + dy²)`) to get the boat-to-segment distance.

The key code concept is:

```text
projection = dot(P − A, B − A) / |B − A|²
projection = clamp(projection, 0, 1)
nearestPoint = A + projection × (B − A)
distance = length(P − nearestPoint)
```

The project repeats that calculation for all 183 segments and keeps the
smallest answer. This prevents it from checking only one small part of the
maritime boundary.

## 7. How the system knows the Boat location

### What it does now

The code currently does **not** connect to a GPS module. `Boat/simulation.h`
defines a list called `simulatedRoute`. Its `readLocation()` function returns
the first item immediately, then returns the next item every five seconds.

For example, a route item is:

```cpp
{20.686100, 92.361000}
```

The first number is latitude and the second is longitude. `boat.ino` stores
this in `currentGpsPoint`, converts it to local metres, finds the nearest
boundary distance, and selects a warning level.

The real physical Boat firmware contains 16 route points. This is still a
simulation source even when uploaded to the ESP32.

### What a real version would do

To make the project know a real boat position, attach a GPS receiver and
replace only the simulated-location implementation. The replacement must keep
the same input shape:

```cpp
struct GPSPoint {
  float latitude;
  float longitude;
};

bool readLocation(GPSPoint *currentGpsPoint);
```

When a valid GPS sentence arrives, `readLocation()` should fill
`currentGpsPoint->latitude` and `currentGpsPoint->longitude`, then return
`true`. The rest of AquaShield—metre conversion, distance calculation,
warnings, and radio packet—can remain unchanged.

## 8. Warning decisions and outputs

Both Boat implementations use these rules:

| Nearest boundary distance | Warning level | Physical Boat output | Wokwi Boat output |
|---:|---|---|---|
| More than 2000 m | SAFE (`0`) | Green LED on; red LED and buzzer off. | Same. |
| 1000–2000 m | WARNING (`1`) | Green off; red LED and buzzer blink every 500 ms. | Green off; Wokwi code currently keeps the red LED/buzzer off in this state. The Serial Monitor still reports `WARNING`. |
| 1000 m or less | DANGER (`2`) | Green off; red LED and buzzer continuously on. | Same continuous red LED and buzzer. |

The difference in the WARNING row is intentional in the current Wokwi code:
Wokwi demonstrates the main state transition and clear danger alarm, while the
physical `warning.h` implements the slow blink. Do not claim the Wokwi warning
LED blinks unless that code is changed.

## 9. Physical Boat program: process in order

This section follows `Boat/boat.ino` exactly.

1. Arduino calls `setup()` one time after power-on/reset.
2. It declares the green LED, red LED, and buzzer pins as outputs and switches
   them off first.
3. It opens the Serial Monitor connection at `115200` baud.
4. `startLocationSource()` resets the simulated route to point zero.
5. `readLocation(&currentGpsPoint)` returns the first prepared coordinate.
6. `updateBoatPosition()` converts longitude to X metres and latitude to Y
   metres, calculates nearest boundary distance, chooses SAFE/WARNING/DANGER,
   and prints the result to Serial Monitor.
7. `startBoatRadio()` starts the ESP32 SPI bus and initializes CC1101 at
   433 MHz, 4.8 kbps, 10 dBm, and 16-bit preamble.
8. Arduino now repeatedly calls `loop()`.
9. In each loop, `readLocation()` returns a new point only when five seconds
   have passed. If not, the prior point remains active.
10. `updateWarningOutputs()` continuously applies the correct LED/buzzer state.
11. Once per second, `sendBoatData()` creates and transmits the current packet.
12. The loop waits 10 ms and repeats.

### Physical radio packet

`Boat/radio.h` sends these fields as binary data:

```text
magic number | boat ID | latitude | longitude | distance in metres | warning level
```

The magic number is `0x41515348`. The receiver checks it, so a random radio
message is less likely to be accepted as AquaShield data. The Boat ID is `01`.
The warning values are `0` SAFE, `1` WARNING, and `2` DANGER.

## 10. Physical Coast Guard program: process in order

This section follows `CoastGuard/receiver.ino` exactly.

1. Arduino calls `setup()` after power-on.
2. It turns the Coast Guard buzzer off and starts Serial at `115200` baud.
3. `startDisplay()` starts I2C on SDA GPIO 4 and SCL GPIO 16, then shows
   `AquaShield`, `Coast Guard Ready`, and `Waiting for boat...` on the OLED.
4. `startCoastGuardRadio()` starts CC1101 with the same radio settings as Boat.
5. It tells RadioLib to call a tiny interrupt function when a packet arrives;
   that function merely sets `radioPacketReceived = true`.
6. In `loop()`, `receiveBoatData()` acts only when that flag is true. It reads
   the bytes, returns the radio to receive mode, checks the magic number,
   checks that warning level is from 0 to 2, and safely ends the Boat ID text.
7. For a valid packet, `showBoatData()` redraws the OLED using the new values.
8. `updateCoastGuardBuzzer()` turns the buzzer on only for DANGER.
9. It prints the received distance and status to the Serial Monitor.

## 11. Wokwi Boat simulation: process in order

`Wokwi/Boat/sketch.ino` contains a self-contained ESP32 simulation. It copies
the real boundary data but has its own code because it uses Wi-Fi/MQTT instead
of RadioLib/CC1101.

1. `setup()` starts the three output pins and Serial Monitor.
2. It calls `calculateCurrentRisk()` for the first simulated GPS point.
3. It connects to Wokwi's special Wi-Fi network, `Wokwi-GUEST`.
4. It tells PubSubClient to use MQTT broker `broker.emqx.io` on port `1883`.
5. In `loop()`, `ensureMqttConnection()` connects/reconnects and gives this
   simulated board a unique client ID based on its ESP32 chip ID.
6. At time 0, 5, 10, 15, 20, and 25 seconds it selects a route point; at 30
   seconds it loops back to the first point.
7. For each point it converts latitude/longitude to metres, checks all 183
   segments, chooses a warning, updates LEDs/buzzer, and prints a line.
8. Every second it publishes a text message such as:

   ```text
   01,20.686100,92.361000,886,2
   ```

   The five comma-separated fields are Boat ID, latitude, longitude, distance
   in metres, and warning number.

### Six-point Wokwi route

| Time | GPS coordinate | Expected distance | Expected state | What to notice |
|---:|---|---:|---|---|
| 0 s | 20.670400, 92.376200 | 2245 m | SAFE | Green LED on. |
| 5 s | 20.681000, 92.365600 | 1625 m | WARNING | Serial status changes to WARNING. |
| 10 s | 20.686100, 92.361000 | 886 m | DANGER | Red LED/buzzer turn on; MQTT sends danger. |
| 15 s | 20.689300, 92.358000 | 413 m | DANGER | Remains in danger, closer to boundary. |
| 20 s | 20.682700, 92.364067 | 1379 m | WARNING | Danger outputs stop; Serial reports WARNING. |
| 25 s | 20.673050, 92.373550 | 2236 m | SAFE | Green LED returns. |

The output at 30 seconds is the first row again. Values are approximate because
the code calculates with floating-point arithmetic and rounds printed metres.

## 12. Wokwi Coast Guard simulation: process in order

`Wokwi/CoastGuard/sketch.ino` is an ESP32 stand-in, not the real ESP8266
firmware.

1. It starts the buzzer and Serial Monitor.
2. It starts I2C on ESP32 GPIO 21 (SDA) and GPIO 22 (SCL), then initializes
   the simulated SSD1306 OLED at address `0x3C`.
3. It shows `AquaShield` and `Waiting for boat...`.
4. It joins `Wokwi-GUEST`, configures MQTT, and connects with a unique client
   ID.
5. When it connects it subscribes to the exact topic:

   ```text
   aquashield/wokwi/v1/boat-status
   ```

6. Whenever a message arrives, `receiveStatus()` first protects itself from an
   oversized message, copies the message into a text buffer, and parses five
   comma-separated values with `sscanf`.
7. It rejects messages that do not have exactly five fields or whose warning
   number is outside 0–2.
8. A valid message becomes a `BoatStatus` object. The OLED is redrawn, the
   Serial Monitor prints `RECEIVED: ...`, and the buzzer turns on only for
   DANGER.

Both Wokwi sketches must use the same MQTT topic. If another group is also
using the public default topic, change `MQTT_TOPIC` in **both** files to one
unique value, for example `aquashield/team7/boat-status`, rebuild both, and
then run both simulations.

## 13. Wokwi circuit connections

### Boat diagram

| Component | ESP32 connection | Purpose |
|---|---|---|
| Green LED | GPIO 26 through 220-ohm resistor, then GND | SAFE indicator. |
| Red LED | GPIO 27 through 220-ohm resistor, then GND | DANGER indicator. |
| Buzzer | GPIO 25 to buzzer, then GND | DANGER sound. |

### Coast Guard diagram

| Component | ESP32 connection | Purpose |
|---|---|---|
| SSD1306 OLED power | 3V3 and GND | Powers display. |
| SSD1306 OLED data | GPIO 21 SDA, GPIO 22 SCL | I2C communication. |
| Buzzer | GPIO 25 to buzzer, then GND | Sounds only for DANGER. |

## 14. How to run the demonstration

Use [setup.md](setup.md) for a fresh Windows/WSL2 laptop. After software is
installed, the concise run order is:

```bash
cd ~/AquaShield/Wokwi/CoastGuard
pio run
cd ~/AquaShield/Wokwi/Boat
pio run
```

Then open `Wokwi/CoastGuard` in one VS Code WSL window and `Wokwi/Boat` in a
second VS Code WSL window. In each window open `diagram.json`, run
**Wokwi: Start Simulator**, start Coast Guard first, then start Boat.

Watch both Serial Monitors. At about 10 seconds, the Boat prints `DANGER` and
transmits its status. The Coast Guard prints `RECEIVED: boat 01 | ... | DANGER`,
the OLED shows the values, and its buzzer turns on.

## 15. A simple presentation script

You can say this in your own words:

> AquaShield warns a boat before it gets too close to a maritime boundary. The
> Boat side receives a position, converts it to metres, checks the shortest
> distance against all 183 pieces of the imported boundary, and chooses safe,
> warning, or danger. It sends its ID, GPS coordinate, measured distance, and
> status to the Coast Guard side. The Coast Guard displays the data and alarms
> during danger. For this presentation, I use six simulated GPS positions in
> Wokwi. The physical design uses an ESP32, an ESP8266, and CC1101 radio; Wokwi
> uses MQTT as a virtual radio because Wokwi cannot simulate that exact radio.

If asked **“How does it know where the boat is?”**, answer:

> In this demonstration, the location is a controlled simulated GPS route.
> The program is designed so that a real GPS reader can replace that one part
> later without changing the boundary-distance, alert, or communication logic.

If asked **“Is it legal navigation equipment?”**, answer:

> No. It is a university warning prototype. It needs real GPS validation,
> approved maritime data, geodesic/GIS processing, reliability testing, and
> certification before any real navigation or enforcement use.

## 16. Important things not to change by accident

- Do not upload `Wokwi/Boat/sketch.ino` to a physical ESP32. Use
  `Boat/boat.ino` for physical Boat hardware.
- Do not upload `Wokwi/CoastGuard/sketch.ino` to a physical NodeMCU. Use
  `CoastGuard/receiver.ino` for physical Coast Guard hardware.
- Do not edit `segments.h` by hand. Its 183 segments are generated data. Keep
  the count correct if the source boundary is regenerated.
- Keep the real Boat and Coast Guard CC1101 settings identical: frequency,
  bit rate, preamble length, and magic number.
- Keep the two Wokwi `MQTT_TOPIC` values identical.
- Re-run `pio run` for each Wokwi project after changing its source, because
  Wokwi runs `firmware.bin`, not the `.ino` file directly.
- Never power a physical CC1101 module from 5 V; it is a 3.3 V device.

## 17. Where to read next

- [README.md](README.md): physical hardware, Arduino IDE installation, wiring,
  and physical upload procedure.
- [Wokwi/README.md](Wokwi/README.md): short Wokwi-only run guide.
- [setup.md](setup.md): step-by-step Windows + WSL2 presentation setup.
