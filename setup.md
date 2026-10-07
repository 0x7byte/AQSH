# AquaShield setup on a friend's Windows laptop (WSL2 + Wokwi)

Use this guide when you need to present AquaShield on a Windows laptop. It runs
the **two supplied Wokwi simulations**: Boat and Coast Guard. No ESP32,
NodeMCU, CC1101, OLED, Arduino IDE, or USB driver is needed for this simulated
presentation.

You do need a stable internet connection: Wokwi uses an online simulator and
the two simulations exchange status through the public MQTT broker
`broker.emqx.io`.

## What to install

Install these before presentation day. The first two are required. The last
three are VS Code extensions; install them in the **WSL: Ubuntu** VS Code
window, not only in a normal Windows window.

| Item | Why it is needed | Official link |
|---|---|---|
| WSL2 with Ubuntu | Linux terminal where PlatformIO builds the firmware. | [Microsoft: install WSL](https://learn.microsoft.com/windows/wsl/install) |
| Visual Studio Code for Windows | Editor and Wokwi simulator interface. | [Download VS Code](https://code.visualstudio.com/download) |
| **WSL** extension (`ms-vscode-remote.remote-wsl`) | Lets VS Code open the project inside Ubuntu. | [VS Code WSL guide](https://code.visualstudio.com/docs/remote/wsl) |
| **Wokwi Simulator** extension (`wokwi.wokwi-vscode`) | Runs the ESP32 circuit simulations in VS Code. | [Wokwi for VS Code](https://docs.wokwi.com/vscode/getting-started) |
| **PlatformIO IDE** extension (`platformio.platformio-ide`) | Helpful for PlatformIO project support and serial/build views. The commands below also work without it. | [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) |
| **C/C++** extension (`ms-vscode.cpptools`) | Optional but recommended for Arduino/C++ code completion and removing false include warnings. | [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) |

You also need a free Wokwi account and a Wokwi VS Code license. After installing
the Wokwi extension, press `F1`, run **Wokwi: Request a new License**, and
follow the browser steps. Do this before the presentation.

## Files to take to the laptop

Copy the complete `AquaShield` folder to a USB drive or cloud storage. Do not
copy only `Boat/` or only one `.ino` file. These are the files used in the
simulation:

~~~text
AquaShield/
├── Wokwi/
│   ├── Boat/
│   │   ├── sketch.ino                 # simulated Boat program
│   │   ├── segments.h                 # real boundary line segments
│   │   ├── diagram.json               # ESP32, LEDs, buzzer circuit
│   │   ├── platformio.ini             # build settings/dependencies
│   │   ├── wokwi.toml                 # points Wokwi to firmware files
│   │   ├── libraries.txt
│   │   └── copy_wokwi_firmware.py
│   └── CoastGuard/
│       ├── sketch.ino                 # simulated Coast Guard program
│       ├── diagram.json               # ESP32, OLED, buzzer circuit
│       ├── platformio.ini
│       ├── wokwi.toml
│       ├── libraries.txt
│       └── copy_wokwi_firmware.py
└── Wokwi/README.md                    # detailed simulation notes
~~~

The existing `firmware.bin` and `firmware.elf` files are included, but rebuild
them on the friend's laptop with the commands below. The two `platformio.ini`
files automatically download the required libraries: PubSubClient, Adafruit
GFX, and Adafruit SSD1306.

## Step 1: install WSL2 and Ubuntu (Windows PowerShell)

Open **PowerShell as Administrator** and run:

~~~powershell
wsl --install -d Ubuntu
~~~

Restart when Windows asks. Open **Ubuntu** from the Start menu, create the
Linux username and password it requests, then check that WSL version 2 is in
use:

~~~powershell
wsl -l -v
~~~

The output should show `Ubuntu` and `VERSION` `2`. If it shows version 1, run:

~~~powershell
wsl --set-version Ubuntu 2
~~~

## Step 2: install VS Code and extensions

1. Install VS Code for Windows from the link above. Keep the option to add
   `code` to PATH if the installer offers it.
2. Open VS Code and install the **WSL** extension.
3. Open Ubuntu, then run this command to open the copied project in a WSL VS
   Code window (the copy steps are in the next section):

   ~~~bash
   code ~/AquaShield
   ~~~

4. Confirm that the bottom-left corner of VS Code says `WSL: Ubuntu`.
5. In that WSL VS Code window, install **Wokwi Simulator**, **PlatformIO IDE**,
   and optionally **C/C++**. If VS Code says an extension is installed locally,
   choose **Install in WSL: Ubuntu**.
6. Press `F1` and complete **Wokwi: Request a new License** while internet is
   available.

## Step 3: copy the project into Ubuntu

First copy `AquaShield` from your USB drive into the Windows Downloads folder,
for example `C:\Users\FriendName\Downloads\AquaShield`.

Then open **Ubuntu** and run these commands one by one. Replace `FriendName`
with the actual Windows account name.

~~~bash
cd /mnt/c/Users/FriendName/Downloads
ls
cp -a AquaShield ~/AquaShield
cd ~/AquaShield
ls
~~~

The final `ls` should show `README.md`, `Wokwi`, `Boat`, and `CoastGuard`.
Working from `~/AquaShield` is important: it is faster and avoids Windows/WSL
file-permission issues compared with building directly in `/mnt/c/...`.

If the folder came from a USB drive instead, locate it first:

~~~bash
ls /mnt
ls /mnt/d
~~~

Then use its actual drive letter, for example:

~~~bash
cp -a /mnt/d/AquaShield ~/AquaShield
~~~

## Step 4: install build tools in Ubuntu

Run these commands in the Ubuntu terminal, one at a time:

~~~bash
sudo apt update
sudo apt install -y python3 python3-venv curl
curl -fsSL -o ~/get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 ~/get-platformio.py
echo 'export PATH="$HOME/.platformio/penv/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
pio --version
~~~

The final command should print a PlatformIO version. This is PlatformIO's
recommended isolated installer; do **not** use `sudo pio`.

## Step 5: build both simulations before presenting

Run each command separately. The first build can take several minutes because
PlatformIO downloads the ESP32 toolchain and libraries.

~~~bash
cd ~/AquaShield/Wokwi/CoastGuard
pio run
~~~

Then build Boat:

~~~bash
cd ~/AquaShield/Wokwi/Boat
pio run
~~~

Both commands must finish with `SUCCESS`. Each build script refreshes the
folder's `firmware.bin` and `firmware.elf`, which Wokwi loads.

Quick check:

~~~bash
ls -lh ~/AquaShield/Wokwi/CoastGuard/firmware.bin ~/AquaShield/Wokwi/Boat/firmware.bin
~~~

## Step 6: start the live Wokwi presentation

1. From Ubuntu, open the project in VS Code:

   ~~~bash
   cd ~/AquaShield
   code .
   ~~~

2. Open a second VS Code window with **File > New Window**. The goal is one
   Wokwi project per window.
3. In window 1, open folder `~/AquaShield/Wokwi/CoastGuard`.
4. In window 2, open folder `~/AquaShield/Wokwi/Boat`.
5. In each window, open `diagram.json`, press `F1`, and choose
   **Wokwi: Start Simulator**.
6. Start **CoastGuard first**, then **Boat**.
7. Open the Wokwi Serial Monitor in both windows. About 10 seconds after Boat
   starts, both monitors should report `DANGER`; the Coast Guard OLED and
   buzzer will also change.

The six-point Boat route restarts automatically after 30 seconds. The full
SAFE → WARNING → DANGER → SAFE demonstration repeats without editing code.

## What to say/show to the teacher

1. Boat begins in `SAFE`; its green LED is on.
2. As the simulated boat approaches the real boundary segments, it changes to
   `WARNING`; the red LED and buzzer blink.
3. At approximately 10 seconds, it reaches `DANGER`; Boat sends its calculated
   status through the simulated MQTT radio link.
4. Coast Guard receives the same Boat ID, coordinates, distance, and status;
   its OLED displays them and its buzzer turns on for `DANGER`.
5. Explain that Wokwi validates the route/risk/transmit/receive/display flow.
   The physical version uses an ESP32 Boat, ESP8266 Coast Guard, and CC1101
   radio modules; Wokwi uses ESP32 on both sides and MQTT because it cannot
   simulate that physical radio link.

## Browser-only fallback (if VS Code/Wokwi extension fails)

Use two browser tabs and [create a new ESP32 Wokwi project](https://wokwi.com/projects/new/esp32) in each. Internet is still required.

| Tab | Replace/add these files from the AquaShield folder |
|---|---|
| Coast Guard | Replace `sketch.ino` and `diagram.json` with `Wokwi/CoastGuard/` versions; add `libraries.txt`. |
| Boat | Replace `sketch.ino` and `diagram.json` with `Wokwi/Boat/` versions; add `libraries.txt` and `segments.h`. |

Start Coast Guard first and Boat second. Do not mix files from the two folders.

## Fast troubleshooting

| Problem | Fix |
|---|---|
| `code: command not found` in Ubuntu | Close and reopen Ubuntu after installing VS Code, then try `code .` again. |
| `pio: command not found` | Run `source ~/.bashrc`, then `pio --version`. If it still fails, use `~/.platformio/penv/bin/pio run`. |
| Red `#include` errors but the build passes | In VS Code run **C/C++: Reset IntelliSense Database**, then reload the VS Code window. |
| Wokwi cannot start | Confirm the Wokwi extension is installed in `WSL: Ubuntu`, then run **Wokwi: Request a new License** again. |
| Boat and Coast Guard do not communicate | Start Coast Guard first; check internet access; wait 10–20 seconds for MQTT reconnection; make sure both sketches retain the same `MQTT_TOPIC`. |
| `pio run` fails while downloading | Connect to internet and repeat the same command. Once both builds pass, leave the laptop online for the Wokwi demo as well. |

For original project details, hardware wiring, and physical-board upload steps,
read [README.md](README.md) and [Wokwi/README.md](Wokwi/README.md).
