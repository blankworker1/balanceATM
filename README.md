# ⚡ Sats ATM


[SatsATM Logo](https://github.com/blankworker1/SatsATM/blob/main/satsatm_logo.svg)

A self-contained coin-to-Lightning machine built with an ESP32, HX-916 coin acceptor, and an Android tablet. Insert coins, enter your Blink username, receive sats instantly in your Lightning wallet.

No backend server. No VPS. Everything runs on the ESP32.

---

## How it works

```
Coins → HX-916 → PC817 → ESP32 → Blink API → visitor's Blink wallet
                              ↓
                       Android tablet (browser)
```

1. Visitor downloads the [Blink app](https://blink.sv/download), creates an account, and sets a username
2. Operator powers on the ESP32 — tablet opens `http://192.168.4.1`
3. On first boot, a settings screen appears — enter WiFi and Blink credentials
4. ESP32 connects to the internet, fetches the EUR→sat rate and starting wallet balance
5. Machine is ready — visitor inserts coins, types their Blink username, receives sats

---

## Repository contents

| File | Purpose |
|---|---|
| `main.ino` | ESP32 firmware (Arduino) |
| `config.h` | Hardware constants and timing values |
| `ui.h` | Tablet UI embedded as PROGMEM string — served by ESP32 |
| `coin_atm_esp32.html` | Source for the tablet UI (edit this, then rebuild `ui.h`) |
|`blink-lookup.html` | Browser tool — paste API key to retrieve correct Wallet ID |
| `README.md` | This file |

---

## Hardware

| Component | Purpose | Cost |
|---|---|---|
| ESP32 dev board (AZDelivery NodeMCU CP2102) | Controller, API client, web server | ~€6 |
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

Install via Arduino Library Manager (**Tools → Manage Libraries**):

- **ArduinoJson** by Benoit Blanchon (v6+)

Everything else (`WiFi`, `WebServer`, `Preferences`, `WiFiClientSecure`) is included with the ESP32 Arduino core.

### Board settings

- Board: `ESP32 Dev Module`
- Partition scheme: `Default 4MB with spiffs` (or any with >1MB app)
- Upload speed: `921600`

### ESP32 Arduino core

If not already installed, add this URL to **Arduino → Preferences → Additional board manager URLs**:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then install `esp32` by Espressif Systems from **Tools → Board → Boards Manager**.

---

## Updating the tablet UI

The tablet UI is embedded directly in `ui.h` as a PROGMEM string — this avoids needing a filesystem on the ESP32.

If you edit `coin_atm_esp32.html`, you need to rebuild `ui.h`:

1. Open `ui.h`
2. Replace everything between `R"rawhtml(` and `)rawhtml"` with the updated HTML
3. Save and reflash

The wrapper structure in `ui.h` must stay exactly as-is:

```cpp
const char UI_HTML[] PROGMEM = R"rawhtml(
... full HTML content here ...
)rawhtml";

String getUI() { return String(FPSTR(UI_HTML)); }
```

---

## First boot / settings

On first power-on (or after a factory reset), the tablet will show the **Settings screen** automatically.

Enter:
- **WiFi SSID** — your phone hotspot or venue network name
- **WiFi password**
- **Blink Wallet ID** — look this up using the wallet ID lookup tool (see below)
- **Blink API Key** — generated at [blink.sv](https://blink.sv) → API Keys (Read + Write permissions)

Press **Save & Connect**. The ESP32 will test the WiFi connection, save credentials to flash, and reboot.

**To access settings again:** hold the ⚡ logo on the tablet screen for 3 seconds.

**Factory reset:** open settings → Factory Reset button (clears all saved credentials).

---

## Blink ATM wallet setup

1. Download the [Blink app](https://blink.sv/download)
2. Create an account and a BTC wallet
3. **Pre-fund the wallet** with enough sats before the event — the machine pays out from this balance
4. Generate an **API Key** at [blink.sv](https://blink.sv) with **Read + Write** permissions (not Receive)
5. Find your **Wallet ID** — it is a long hex string, *not* the same as the API key. Use this query against the Blink API to retrieve it:

```graphql
query {
  me {
    defaultAccount {
      wallets {
        id
        walletCurrency
        balance
      }
    }
  }
}
```

Or open [`blink-lookup.html`](https://blankworker1.github.io/balanceATM/blink-lookup.html) in a browser — paste your API key and it returns the correct wallet ID with one click.

> ⚠️ The wallet is a **treasury** — sats flow out to visitors. The operator collects the physical coins as revenue. Monitor the wallet balance during events using the on-screen fuel gauge.

---

## Visitor flow

1. Download Blink app → create account → set a username (e.g. `alice`)
2. Insert coins into the ATM
3. When prompted, type your Blink username on the tablet (just `alice` — not `alice@blink.sv`)
4. Tap **Send my sats**
5. Sats arrive in your Blink wallet instantly

If the username is wrong, the payment fails and the visitor can retry — coins are not lost.

---

## Coin acceptor configuration

The HX-916 must be programmed to send the correct number of pulses per coin. Default in `config.h`:

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
4. Optional: use [Fully Kiosk Browser](https://www.fully-kiosk.com/) to lock the screen to this URL

---

## Security notes

This is a **proof-of-concept** for operator-supervised events. Known limitations:

| Item | Status |
|---|---|
| TLS certificate verification | Disabled (`setInsecure()`) — PoC only |
| API key stored in ESP32 NVS flash | Accessible if device is physically compromised |
| No authentication on settings screen | Relies on physical AP access |
| Max credit per session | €10 (configurable in `config.h`) |

Do not deploy unattended or at high-value volumes without addressing these points.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Settings screen won't go away | WiFi connection failing | Check SSID/password, ensure hotspot is on |
| Rate shows `—` | Blink API unreachable | Check internet connection on ESP32 |
| Coins not detected | Wrong GPIO or wiring | Check PC817 wiring, confirm `COIN_PIN` in `config.h` |
| Payment failed — wrong username | Visitor mistyped | Retry with correct Blink username |
| Payment failed — API error | Wallet ID or API key wrong | Re-enter credentials in settings |
| Gauge always empty | Balance fetch failing | Check API key has Read permission |
| Wallet balance not updating after payment | Expected — balance deducted locally | Refreshes on next boot or idle poll |

---

## License

MIT — do what you want, don't blame us if it breaks.

---

*Built with: ESP32 · Arduino · Blink Lightning API · vanilla JS · no cloud*
