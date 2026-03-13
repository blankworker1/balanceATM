# Sats ATM

![SatsATM Logo](https://emerald-real-clownfish-172.mypinata.cloud/ipfs/bafkreibnfqjqom5cb5j6hae2ubfx37y2rf6iou2b7iuziwxymkkwkihnrq)

A self-contained coin-to-Lightning machine built with an ESP32, HX-916 coin acceptor, and an Android tablet. Insert coins, enter your wallet LN address, receive sats instantly in your Lightning wallet. Powered by Blink.

No backend server. No VPS. Everything runs on the ESP32.

---

## How it works

```
Coins → HX-916 → PC817 → ESP32 → Blink API → visitor's Blink wallet
                              ↓
                       Android tablet (browser)
                              ↓
                   Victron MPPT → VE.Direct → ESP32
                              ↓
                   Solar monitor at /solar
```

1. Visitor downloads the [Blink app](https://blink.sv/download), creates an account, and sets a username
2. Operator powers on the ESP32 — tablet opens `http://192.168.4.1`
3. On first boot, a settings screen appears — enter WiFi and Blink ATM wallet API credentials
4. ESP32 connects to the internet, fetches the EUR→sat rate and starting wallet balance
5. Machine is ready — visitor inserts coins, types their Blink wallet LN address, receives sats

---

## Repository contents

| File | Purpose |
|---|---|
| `main.ino` | ESP32 firmware (Arduino) |
| `config.h` | Hardware constants and timing values |
| `ui.h` | ATM tablet UI embedded as PROGMEM string |
| `coin_atm_esp32.html` | Source for the tablet UI (edit this, then rebuild `ui.h`) |
| `solar.h` | Solar monitor UI embedded as PROGMEM string |
| `solar.html` | Source for the solar monitor UI (edit this, then rebuild `solar.h`) |
| `blink-lookup.html` | Browser tool — paste API key to retrieve correct Wallet ID |
| `README.md` | This file |

---

## Hardware

### ATM core

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

### Solar power system (optional)

| Component | Purpose |
|---|---|
| Victron SmartSolar MPPT 75/15 | Solar charge controller |
| 12V LiFePO4 battery (6Ah recommended) | Energy storage |
| Solar panel (12V, up to 75W) | Power source |
| DC-DC buck converter (12V → 5V) | Powers tablet + ESP32 from battery |
| JST-PH 2.0mm 4-pin cable | VE.Direct connector |

> The Victron MPPT load output handles low voltage disconnect in hardware — no relay needed. Configure cutoff voltage in the VictronConnect app (recommended: 11.8V cutoff, 13.0V reconnect).

---

## Wiring

### ATM core

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

### VE.Direct (solar monitor)

```
Victron MPPT VE.Direct TX  →  ESP32 GPIO16 (Serial2 RX)
Victron MPPT VE.Direct GND →  ESP32 GND
(Leave RX and VCC unconnected)
```

> ⚠️ Do not connect the VE.Direct VCC pin — it has a 10mA limit and will permanently damage the Victron device. Only TX and GND are needed.

> The SmartSolar 75/15 TX pin operates at 3.3V-compatible levels — no level shifter required.

### Solar power

```
Battery +  →  MPPT LOAD+  →  Buck converter in+
Battery -  →  MPPT LOAD-  →  Buck converter in-
                              Buck converter out+ (5V) → USB → Tablet + ESP32
```

---

## Network

The ESP32 runs in dual WiFi mode simultaneously:

| Mode | SSID | Purpose |
|---|---|---|
| Access Point (AP) | `SatsATM` | Tablet connects here |
| Station (STA) | Your hotspot / router | ESP32 reaches the internet |

The tablet connects to `SatsATM` and opens `http://192.168.4.1`. It never needs its own internet connection — all API calls go through the ESP32.

**Captive portal:** The ESP32 runs a DNS server that catches all DNS queries from the tablet and returns `192.168.4.1`. It also responds to Android/Fire OS connectivity checks (`/generate_204`) with a `204` response. This prevents the tablet from showing "no internet" warnings or dropping the AP connection automatically.

**Recommended internet source:** operator's phone hotspot. Credentials are entered via the settings screen and saved to ESP32 flash — no reflashing needed when moving between events.

---

## Web interface

| URL | Description |
|---|---|
| `http://192.168.4.1/` | ATM visitor interface |
| `http://192.168.4.1/solar` | Solar + system monitor (operator) |
| `http://192.168.4.1/state` | ATM state JSON (polled by tablet) |
| `http://192.168.4.1/solar-data` | Solar data JSON (polled by solar page) |

### Solar monitor (`/solar`)

Displays live data from the Victron MPPT via VE.Direct:

- **Battery voltage** with LiFePO4 state of charge bar (12.0V–13.6V range)
- **Panel power** (W) and panel voltage (V)
- **Charge state** — Bulk / Absorption / Float / Off / Fault with colour coding
- **Today's yield** in Wh
- **Battery current** — positive = charging, negative = discharging

---

## Topbar status indicators

Both the ATM and solar pages show live system status in the topbar:

| Indicator | Green | Yellow | Red |
|---|---|---|---|
| Battery | Above 12.2V | 12.0V–12.2V | Below 12.0V |
| Connectivity | Blink API reachable | — | Blink API unreachable |

- **Battery indicator** — hidden until first VE.Direct frame received
- **Connectivity dot** — pulses green when online, flashes red when offline
- **Low battery banner** — slides up from bottom of ATM screen at warning and critical levels
- **INSERT COINS disabled** — automatically blocked when battery is critical (below 12.0V). Any transaction already in progress completes normally.

---

## Arduino setup

### Required libraries

Install via Arduino Library Manager (**Tools → Manage Libraries**):

- **ArduinoJson** by Benoit Blanchon (v6+)

Everything else (`WiFi`, `WebServer`, `DNSServer`, `Preferences`, `WiFiClientSecure`) is included with the ESP32 Arduino core.

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

The tablet UI is embedded directly in `ui.h` as a PROGMEM string — this avoids needing a filesystem on the ESP32. The solar monitor is embedded the same way in `solar.h`.

If you edit `coin_atm_esp32.html`, rebuild `ui.h`:

1. Open `ui.h`
2. Replace everything between `R"rawhtml(` and `)rawhtml"` with the updated HTML
3. Save and reflash

The wrapper structure must stay exactly as-is:

```cpp
const char UI_HTML[] PROGMEM = R"rawhtml(
... full HTML content here ...
)rawhtml";

String getUI() { return String(FPSTR(UI_HTML)); }
```

Same process applies to `solar.html` → `solar.h` using `SOLAR_HTML` and `getSolarUI()`.

---

## First boot / settings

On first power-on (or after a factory reset), the tablet will show the **Settings screen** automatically.

Enter:
- **WiFi SSID** — your phone hotspot or venue network name
- **WiFi password**
- **Blink Wallet ID** — BTC wallet only (not USD). Use the wallet ID lookup tool (see below)
- **Blink API Key** — generated at [blink.sv](https://blink.sv) → API Keys (Read + Write permissions)
- **Pulses per Euro** — must match your HX-916 DIP switch setting (default: 10)

Press **Save & Connect**. The ESP32 will test the WiFi connection, save credentials to flash, and reboot.

After rebooting, watch the **Serial Monitor** at 115200 baud — if this is the first boot, insert a coin when prompted to complete coin polarity auto-detection. The result is saved to flash and never needs repeating unless you do a factory reset.

**To access settings again:** hold the ⚡ logo on the tablet screen for 3 seconds, then enter PIN **1928**.

**Factory reset:** open settings → Factory Reset button. This clears all saved credentials including coin polarity — polarity detection will run again on next boot.

---

## Coin polarity auto-detection

The HX-916 signal polarity (active-HIGH or active-LOW) varies between hardware revisions. On first boot the ESP32 auto-detects the correct polarity and saves it to NVS flash.

1. Boot the ESP32 and open Serial Monitor (115200 baud)
2. When prompted, insert a single coin within 15 seconds
3. Polarity is detected, saved, and used for all future boots
4. If no coin is inserted within 15 seconds, active-LOW is assumed (correct for most units)

To re-run detection: Factory Reset in settings → reboot → insert coin when prompted.

---

## Blink ATM wallet setup

1. Download the [Blink app](https://blink.sv/download)
2. Create an account and a BTC wallet
3. **Pre-fund the wallet** with enough sats before the event — the machine pays out from this balance
4. Generate an **API Key** at [blink.sv](https://blink.sv) with **Read + Write** permissions (not Receive)
5. Find your **BTC Wallet ID** (not the USD wallet) — use this query against the Blink API:

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

The pulses per Euro value can also be set in the settings screen without reflashing — useful when moving between events with different coin configurations.

---

## Tablet setup

1. Factory reset the tablet if repurposing an existing device
2. Connect tablet WiFi to `SatsATM` (password: `lightning`) — ignore any "no internet" warning and confirm to stay connected
3. Open Silk Browser (or Chrome) and navigate to `http://192.168.4.1` — the ESP32 captive portal DNS will handle this automatically on most devices
4. Set screen timeout to **Never**: Settings → Display → Sleep → Never
5. Optional: use [Fully Kiosk Browser](https://www.fully-kiosk.com/) or a custom kiosk APK to lock the screen to this URL

> **Note for Amazon Fire tablets:** Fire OS aggressively drops WiFi connections with no internet gateway. The ESP32 captive portal (DNS + `/generate_204`) resolves this automatically — no ADB commands or rooting required.

---

## Security notes

This is a **proof-of-concept** for operator-supervised events. Known limitations:

| Item | Status |
|---|---|
| TLS certificate verification | Disabled (`setInsecure()`) — PoC only |
| API key stored in ESP32 NVS flash | Accessible if device is physically compromised |
| Settings PIN protection | 3-second logo hold + PIN 1928 |
| AP limited to 1 client | Prevents race conditions and double payments |
| Max credit per session | €10 (configurable in `config.h`) |

Do not deploy unattended or at high-value volumes without addressing the TLS point.

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
| Fire tablet drops SatsATM WiFi | Fire OS internet check failing | Reflash latest `main.ino` — captive portal fix included |
| Solar page shows no data | VE.Direct not connected | Check GPIO16 → MPPT TX wiring |
| Solar page shows stale data | VE.Direct cable issue | Check GND connection, verify MPPT is powered |
| Battery indicator not showing | No VE.Direct frames received yet | Wait for first frame — appears within 2 seconds of wiring |
| INSERT COINS disabled | Battery critical (<12.0V) | Charge battery — Victron load output will reconnect at 13.0V |
| Connectivity dot red | Blink API unreachable | Check modem/hotspot internet connection |

---

## License

MIT — do what you want, don't blame us if it breaks.

---

*Built with: ESP32 · Arduino · Blink Lightning API · Victron VE.Direct · vanilla JS · no cloud*
