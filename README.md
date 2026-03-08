# ⚡ Sats ATM

A self-contained coin-to-Lightning machine built with an ESP32, HX-916 coin acceptor, and an Android tablet. Insert coins, scan a QR code with a Lightning wallet, receive sats instantly.

No backend server. No VPS. No exchange-rate API. Everything runs on the ESP32.

---

## How it works

```
Coins → HX-916 → PC817 → ESP32 → Blink API → Lightning payment
                              ↓
                       Android tablet (browser)
```

1. Operator powers on the ESP32 and opens `http://192.168.4.1` on the tablet
2. On first boot, a settings screen appears — enter WiFi and Blink credentials
3. ESP32 connects to the internet, fetches the EUR→sat rate and starting wallet balance
4. Machine is ready — buyer inserts coins, scans the QR, receives sats

[Link to Demo version](

---

## Repository contents

| File | Purpose |
|---|---|
| `main.ino` | ESP32 firmware (Arduino) |
| `config.h` | Hardware constants and timing values |
| `coin_atm_esp32.html` | Tablet UI — served directly by the ESP32 |
| `coin_atm_demo.html` | Interactive demo mockup (for GitHub Pages / testing) |
| `README.md` | This file |

---

## Hardware

| Component | Purpose | Cost |
|---|---|---|
| ESP32 dev board | Controller, API client, web server | ~€6 |
| HX-916 coin acceptor | Accepts and validates coins | ~€15 |
| PC817 optocoupler | Isolates 12V HX-916 signal from 3.3V ESP32 | ~€0.30 |
| 12V DC power supply | Powers HX-916 | ~€8 |
| 1kΩ resistor | Current limiter — PC817 input side | ~€0.05 |
| 10kΩ resistor | Pulldown — PC817 output side | ~€0.05 |
| Android tablet | Display (browser kiosk) | Existing |

**Total new hardware cost: ~€30**

---

## Wiring

```
── 12V side (HX-916) ──────────────────────────────────
HX-916 RED    →  12V supply
HX-916 BLACK  →  GND (shared with ESP32 GND)
HX-916 WHITE  →  1kΩ resistor  →  PC817 pin 1 (anode)
                                   PC817 pin 2 (cathode)  →  GND

── 3.3V side (ESP32) ───────────────────────────────────
PC817 pin 4 (collector)  →  3.3V
PC817 pin 3 (emitter)    →  ESP32 GPIO4
                         →  10kΩ pulldown  →  GND
```

> ⚠️ The 12V and 3.3V sides share a common GND. Do not connect any other pins between sides.

---

## Network

The ESP32 runs in dual WiFi mode simultaneously:

| Mode | SSID | Purpose |
|---|---|---|
| Access Point (AP) | `SatsATM` | Tablet connects here |
| Station (STA) | Your hotspot / router | ESP32 reaches the internet |

The tablet connects to `SatsATM` and opens `http://192.168.4.1`. It never needs its own internet connection — all API calls go through the ESP32.

**Recommended internet source:** operator's phone hotspot. Credentials are entered via the settings screen and saved to ESP32 flash — no reflashing needed when moving between events.

---

## Arduino setup

### Required libraries

Install via Arduino Library Manager:

- **ArduinoJson** by Benoit Blanchon (v6+)
- **WebServer** — included with ESP32 Arduino core

### Board

- Board: `ESP32 Dev Module`
- Partition scheme: `Default 4MB with spiffs` (or any with >1MB app)
- Upload speed: `921600`

### ESP32 Arduino core

If not already installed, add this URL to Arduino → Preferences → Additional board manager URLs:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then install `esp32` by Espressif Systems from Board Manager.

---

## Serving the HTML from the ESP32

The tablet UI (`coin_atm_esp32.html`) needs to be embedded in the firmware. The simplest approach for a PoC is to store it as a PROGMEM string in a header file:

1. Copy the contents of `coin_atm_esp32.html`
2. Create `ui.h` in your sketch folder
3. Paste as a raw string:

```cpp
// ui.h
#pragma once
const char UI_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
... paste full HTML here ...
)rawhtml";

String getUI() { return String(FPSTR(UI_HTML)); }
```

4. The `main.ino` `server.on("/")` handler calls `getUI()` to serve it

> For larger deployments, use LittleFS to upload the HTML as a separate file. See the ESP32 LittleFS documentation for the upload tool.

---

## First boot / settings

On first power-on (or after a factory reset), the tablet will show the **Settings screen** automatically.

Enter:
- **WiFi SSID** — your phone hotspot or venue network name
- **WiFi password**
- **Blink Wallet ID** — found in the Blink app under wallet settings
- **Blink API Key** — generated at [blink.sv](https://blink.sv) → API Keys

Press **Save & Connect**. The ESP32 will test the WiFi connection, save credentials to flash, and reboot. The machine is then ready.

**To access settings again:** hold the ⚡ logo on the tablet screen for 3 seconds.

**Factory reset:** open settings → Factory Reset button (clears all saved credentials).

---

## Blink wallet setup

1. Download the [Blink app](https://blink.sv/download)
2. Create an account and a BTC wallet
3. **Pre-fund the wallet** with enough sats before the event — the machine dispenses from this balance
4. Copy your **Wallet ID** from the app settings
5. Generate an **API Key** at [blink.sv](https://blink.sv) with read + write permissions
6. Enter both in the ATM settings screen

> ⚠️ The wallet is a **treasury** — sats flow out to buyers. The operator collects the physical coins as revenue. Monitor the wallet balance during events using the on-screen fuel gauge.

---

## Coin acceptor configuration

The HX-916 must be programmed to send the correct number of pulses per coin. Default configuration in `config.h`:

```
PULSES_PER_EUR = 10
```

| Coin | Pulses |
|---|---|
| €0.50 | 5 |
| €1.00 | 10 |
| €2.00 | 20 |

Reprogram the HX-916 using its built-in programming mode (see HX-916 datasheet) if your pulse counts differ.

---

## Tablet setup

1. Connect tablet WiFi to `SatsATM` (password: `lightning`)
2. Open Chrome and navigate to `http://192.168.4.1`
3. Set screen timeout to **Never**: Settings → Display → Sleep → Never
4. Optional: use [Fully Kiosk Browser](https://www.fully-kiosk.com/) to lock the screen to this URL and prevent accidental navigation

---

## Security notes

This is a **proof-of-concept** for operator-supervised events. Known limitations:

| Item | Status |
|---|---|
| TLS certificate verification | Disabled (`setInsecure()`) — PoC only |
| API key stored in ESP32 NVS flash | Accessible if device is physically compromised |
| No authentication on settings screen | Relies on physical AP access |
| Max credit limit | €10 per session (configurable in `config.h`) |

Do not deploy unattended or at high-value volumes without addressing these points.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Settings screen won't go away | WiFi connection failing | Check SSID/password, ensure hotspot is on |
| Rate shows `—` | Blink API unreachable | Check internet connection on ESP32 |
| Coins not detected | Wrong GPIO or wiring | Check PC817 wiring, confirm `COIN_PIN` in config.h |
| QR not generating | Invoice creation failed | Check Wallet ID and API Key in settings |
| Gauge always empty | Balance fetch failing | Check API Key has read permission |

---

## License

MIT — do what you want, don't blame us if it breaks.

---

*Built with: ESP32 · Arduino · Blink Lightning API · vanilla JS · no cloud*
