// ═══════════════════════════════════════════════════════════════
//  main.ino  —  Sats ATM firmware
//
//  Hardware:  ESP32 + HX-916 coin acceptor via PC817 optocoupler
//  Display:   Android tablet (browser at http://192.168.4.1)
//  API:       Blink Lightning wallet (GraphQL over HTTPS)
//
//  Flow:
//    Boot → load saved credentials → connect WiFi (STA+AP)
//         → fetch rate + starting balance
//         → serve web UI → accept coins → visitor enters username
//         → ESP32 sends sats to visitor's Blink address → reset
//
//  First boot (no saved credentials):
//    Boot → AP only (single client) → serve settings screen
//         → operator configures → save to NVS → reboot
//         → coin polarity detection → normal flow
//
//  Security:
//    - AP limited to 1 client at a time
//    - State machine prevents double payments
//    - API call timeout prevents frozen sending screen
//    - Coin polarity auto-detected and saved to NVS
// ═══════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "config.h"
#include "ui.h"

// ── Objects ──────────────────────────────────────────────────────
WebServer    server(80);
Preferences  prefs;
WiFiClientSecure client;

// ── Saved credentials (loaded from NVS on boot) ──────────────────
String savedSSID;
String savedPassword;
String savedWalletId;
String savedApiKey;
int    savedPulsesPerEur = PULSES_PER_EUR;  // fallback to config.h default

// ── Runtime state ────────────────────────────────────────────────
enum State { IDLE, COINS_INSERTED, READY, SENDING, PAID, ERROR_STATE };
volatile State  appState       = IDLE;
volatile int    pulseCount     = 0;
volatile ulong  lastPulseTime  = 0;

float   creditEUR        = 0.0f;
int     creditSats       = 0;
int     satsPerEUR       = 0;
long    startingBalance  = 0;
long    currentBalance   = 0;
String  lastError        = "";
ulong   lastBalancePoll  = 0;
bool    wifiConnected    = false;
bool    settingsMode     = false;

// ── Coin ISR ─────────────────────────────────────────────────────
void IRAM_ATTR onCoinPulse() {
  if (appState == IDLE || appState == COINS_INSERTED) {
    if (pulseCount < MAX_PULSES) pulseCount++;
    lastPulseTime = millis();
    appState = COINS_INSERTED;
  }
}

// ═══════════════════════════════════════════════════════════════
//  COIN POLARITY AUTO-DETECT
//  Runs on first boot only — result saved to NVS flash.
//  Insert a coin when prompted via Serial Monitor.
//  Returns true = active-LOW (INPUT_PULLUP + FALLING)
//          false = active-HIGH (INPUT_PULLDOWN + RISING)
// ═══════════════════════════════════════════════════════════════
bool detectCoinPolarity() {
  // Check if already saved from a previous boot
  prefs.begin("satm", false);
  bool saved = prefs.getBool("polSaved", false);
  if (saved) {
    bool activeLow = prefs.getBool("activeLow", true);
    prefs.end();
    Serial.printf("Coin polarity loaded from flash: %s\n",
                  activeLow ? "ACTIVE_LOW (PULLUP+FALLING)"
                            : "ACTIVE_HIGH (PULLDOWN+RISING)");
    return activeLow;
  }
  prefs.end();

  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("FIRST BOOT — Coin polarity detection");
  Serial.println("Insert a coin within 15 seconds...");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

  bool activeLow = true;  // safe default
  bool detected  = false;
  unsigned long start = millis();

  while (millis() - start < 15000) {
    // Test active-LOW (PULLUP)
    pinMode(COIN_PIN, INPUT_PULLUP);
    delay(10);
    if (digitalRead(COIN_PIN) == LOW) {
      activeLow = true;
      detected  = true;
      Serial.println("✓ Detected ACTIVE_LOW — using INPUT_PULLUP + FALLING");
      break;
    }
    // Test active-HIGH (PULLDOWN)
    pinMode(COIN_PIN, INPUT_PULLDOWN);
    delay(10);
    if (digitalRead(COIN_PIN) == HIGH) {
      activeLow = false;
      detected  = true;
      Serial.println("✓ Detected ACTIVE_HIGH — using INPUT_PULLDOWN + RISING");
      break;
    }
  }

  if (!detected) {
    Serial.println("No coin detected within 15s — defaulting to ACTIVE_LOW");
    Serial.println("You can re-run detection via Factory Reset in settings");
    activeLow = true;
  }

  // Save to NVS so we never need to detect again
  prefs.begin("satm", false);
  prefs.putBool("polSaved", true);
  prefs.putBool("activeLow", activeLow);
  prefs.end();
  Serial.println("Coin polarity saved to flash ✓");

  return activeLow;
}

// ═══════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n⚡ Sats ATM booting...");

  // ── Load saved credentials ──────────────────────────────────
  prefs.begin("satm", false);
  savedSSID         = prefs.getString("ssid",       "");
  savedPassword     = prefs.getString("password",   "");
  savedWalletId     = prefs.getString("walletId",   "");
  savedApiKey       = prefs.getString("apiKey",     "");
  savedPulsesPerEur = prefs.getInt("pulsesPerEur",  PULSES_PER_EUR);
  prefs.end();

  // ── Start AP — limited to 1 client for security ──────────────
  // Parameters: SSID, password, channel, hidden, max_connection
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD, 1, 0, 1);
  Serial.printf("AP started: %s  IP: %s  (max 1 client)\n", AP_SSID, AP_IP);

  // ── TLS: PoC mode — operator supervised ──────────────────────
  client.setInsecure();

  // ── Try STA connection if credentials saved ───────────────────
  if (savedSSID.length() > 0 && savedWalletId.length() > 0) {
    Serial.printf("Connecting to WiFi: %s\n", savedSSID.c_str());
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500); Serial.print("."); attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.printf("\nConnected. IP: %s\n", WiFi.localIP().toString().c_str());
      fetchBootData();
    } else {
      Serial.println("\nWiFi connection failed — showing settings");
      settingsMode = true;
    }
  } else {
    Serial.println("No credentials saved — showing settings screen");
    settingsMode = true;
  }

  // ── Coin polarity auto-detect then attach interrupt ───────────
  bool activeLow = detectCoinPolarity();
  pinMode(COIN_PIN, activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(COIN_PIN), onCoinPulse,
                  activeLow ? FALLING : RISING);
  Serial.printf("Coin interrupt attached on GPIO%d\n", COIN_PIN);

  // ── Web server routes ─────────────────────────────────────────
  setupRoutes();
  server.begin();
  Serial.println("Web server started");
}

// ═══════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════
void loop() {
  server.handleClient();

  if (settingsMode || !wifiConnected) return;

  // ── Coin timeout check ────────────────────────────────────────
  if (appState == COINS_INSERTED &&
      (millis() - lastPulseTime) > INTER_COIN_MS) {
    float euros = (float)pulseCount / (float)savedPulsesPerEur;
    if (euros > MAX_CREDIT_EUR) euros = MAX_CREDIT_EUR;
    creditEUR  = euros;
    creditSats = (int)(euros * satsPerEUR);
    pulseCount = 0;
    lastError  = "";
    Serial.printf("Coins finalised: €%.2f = %d sats\n", creditEUR, creditSats);
    appState = READY;
  }

  // ── Poll wallet balance ───────────────────────────────────────
  if (appState == IDLE) {
    if ((millis() - lastBalancePoll) > BALANCE_POLL_MS) {
      lastBalancePoll = millis();
      fetchBalance();
    }
  }

  // ── Auto-reset after PAID ─────────────────────────────────────
  static ulong paidTime = 0;
  if (appState == PAID) {
    if (paidTime == 0) paidTime = millis();
    if ((millis() - paidTime) > 5000) {
      resetState();
      paidTime = 0;
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  BOOT DATA FETCH
// ═══════════════════════════════════════════════════════════════
void fetchBootData() {
  Serial.println("Fetching boot data from Blink...");
  satsPerEUR      = fetchSatsPerEUR();
  startingBalance = fetchBalance();
  currentBalance  = startingBalance;
  Serial.printf("Rate: 1 EUR = %d sats\n", satsPerEUR);
  Serial.printf("Starting balance: %ld sats\n", startingBalance);
}

// ── Fetch EUR → sats rate ─────────────────────────────────────
int fetchSatsPerEUR() {
  String query = "{\"query\":\"{ btcPriceList(range: ONE_DAY) { price { base offset } } }\"}";
  String resp  = blinkPost(query);
  if (resp.length() == 0) return 1600;

  StaticJsonDocument<1024> doc;
  if (deserializeJson(doc, resp)) return 1600;

  float btcUSD = doc["data"]["btcPriceList"][0]["price"]["base"].as<float>();
  int   offset = doc["data"]["btcPriceList"][0]["price"]["offset"].as<int>();
  while (offset < 0) { btcUSD /= 10.0f; offset++; }
  int sats = (int)(100000000.0f / btcUSD);
  return (sats > 0) ? sats : 1600;
}

// ── Fetch wallet balance ──────────────────────────────────────
long fetchBalance() {
  String query = "{\"query\":\"{ me { defaultAccount { wallets { balance } } } }\","
                 "\"operationName\":null}";
  String resp  = blinkPost(query);
  if (resp.length() == 0) return currentBalance;

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, resp)) return currentBalance;

  JsonArray wallets = doc["data"]["me"]["defaultAccount"]["wallets"];
  for (JsonObject w : wallets) {
    if (w["balance"].as<long>() >= 0) {
      currentBalance = w["balance"].as<long>();
      return currentBalance;
    }
  }
  return currentBalance;
}

// ═══════════════════════════════════════════════════════════════
//  SEND SATS TO VISITOR
// ═══════════════════════════════════════════════════════════════
bool sendToLightningAddress(const String& lnAddress, int amountSats) {
  String query = String("{\"query\":\"mutation { lnAddressPaymentSend(input: { "
                        "walletId: \\\"") + savedWalletId + "\\\" "
                        "lnAddress: \\\"" + lnAddress    + "\\\" "
                        "amount: "        + String(amountSats) +
                        " }) { status errors { message } } }\"}";

  Serial.printf("Sending %d sats to %s\n", amountSats, lnAddress.c_str());

  // ── 15 second timeout guard ───────────────────────────────────
  unsigned long sendStart = millis();

  String resp = blinkPost(query);

  if (millis() - sendStart > 15000) {
    lastError = "Payment timed out — please retry";
    Serial.println("Send failed: timeout");
    return false;
  }

  if (resp.length() == 0) {
    lastError = "No response from Blink API";
    Serial.println("Send failed: no response");
    return false;
  }

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, resp)) {
    lastError = "Invalid API response";
    Serial.println("Send failed: JSON parse error");
    return false;
  }

  if (doc.containsKey("errors")) {
    lastError = doc["errors"][0]["message"].as<String>();
    Serial.printf("Send failed (GraphQL error): %s\n", lastError.c_str());
    return false;
  }

  String status = doc["data"]["lnAddressPaymentSend"]["status"].as<String>();

  JsonArray errors = doc["data"]["lnAddressPaymentSend"]["errors"];
  if (errors.size() > 0) {
    lastError = errors[0]["message"].as<String>();
    Serial.printf("Send failed (mutation error): %s\n", lastError.c_str());
    return false;
  }

  if (status == "SUCCESS") {
    Serial.printf("Payment SUCCESS: %d sats → %s\n", amountSats, lnAddress.c_str());
    currentBalance -= amountSats;
    return true;
  }

  lastError = "Payment status: " + status;
  Serial.printf("Send failed: status=%s\n", status.c_str());
  return false;
}

// ═══════════════════════════════════════════════════════════════
//  BLINK HTTP POST
// ═══════════════════════════════════════════════════════════════
String blinkPost(const String& payload) {
  if (!client.connect("api.blink.sv", 443)) {
    Serial.println("Blink connect failed");
    return "";
  }

  client.println("POST /graphql HTTP/1.1");
  client.println("Host: api.blink.sv");
  client.println("Content-Type: application/json");
  client.println("X-API-KEY: " + savedApiKey);
  client.println("Content-Length: " + String(payload.length()));
  client.println("Connection: close");
  client.println();
  client.print(payload);

  ulong t = millis();
  while (!client.available() && millis() - t < 10000) delay(10);

  while (client.connected() || client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String body = "";
  while (client.available()) body += (char)client.read();
  client.stop();
  return body;
}

// ═══════════════════════════════════════════════════════════════
//  STATE RESET
// ═══════════════════════════════════════════════════════════════
void resetState() {
  appState   = IDLE;
  pulseCount = 0;
  creditEUR  = 0.0f;
  creditSats = 0;
  lastError  = "";
  Serial.println("Session reset → IDLE");
}

// ═══════════════════════════════════════════════════════════════
//  WEB SERVER ROUTES
// ═══════════════════════════════════════════════════════════════
void setupRoutes() {

  // ── Main UI ────────────────────────────────────────────────
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getUI());
  });

  // ── State endpoint (tablet polls every 2s) ────────────────
  server.on("/state", HTTP_GET, []() {
    String gaugeLevel = "high";
    if (startingBalance > 0) {
      float pct = (float)currentBalance / (float)startingBalance;
      if      (pct <= 0.05f)       gaugeLevel = "critical";
      else if (pct <= GAUGE_LOW)   gaugeLevel = "low";
      else if (pct <= GAUGE_MID)   gaugeLevel = "mid";
      else if (pct <= GAUGE_HIGH)  gaugeLevel = "high";
    }

    String stateStr;
    switch (appState) {
      case IDLE:          stateStr = "IDLE";    break;
      case COINS_INSERTED:stateStr = "COINS";   break;
      case READY:         stateStr = "READY";   break;
      case SENDING:       stateStr = "SENDING"; break;
      case PAID:          stateStr = "PAID";    break;
      default:            stateStr = "ERROR";   break;
    }

    String json = "{";
    json += "\"state\":\""      + stateStr                           + "\",";
    json += "\"sats\":"         + String(creditSats)                 + ",";
    json += "\"credit\":"       + String(creditEUR, 2)               + ",";
    json += "\"gauge\":\""      + gaugeLevel                         + "\",";
    json += "\"rate\":"         + String(satsPerEUR)                 + ",";
    json += "\"needsSetup\":"   + String(settingsMode ? "true" : "false");
    json += "}";

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
  });

  // ── Send endpoint ─────────────────────────────────────────
  server.on("/send", HTTP_POST, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (appState != READY) {
      server.send(409, "application/json",
        "{\"success\":false,\"error\":\"Not ready — insert coins first\"}");
      return;
    }

    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"no body\"}");
      return;
    }

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"invalid json\"}");
      return;
    }

    String lnAddress = doc["lnAddress"] | "";
    if (lnAddress.length() == 0) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"lnAddress required\"}");
      return;
    }

    if (lnAddress.indexOf('@') < 0) {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid Lightning address\"}");
      return;
    }

    appState = SENDING;

    bool ok = sendToLightningAddress(lnAddress, creditSats);

    if (ok) {
      appState = PAID;
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      appState = READY;
      String errJson = "{\"success\":false,\"error\":\"" + lastError + "\"}";
      server.send(200, "application/json", errJson);
    }
  });

  // ── Settings save endpoint ────────────────────────────────
  server.on("/settings", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"no body\"}");
      return;
    }

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
      server.send(400, "application/json", "{\"error\":\"invalid json\"}");
      return;
    }

    String newSSID       = doc["ssid"]         | "";
    String newPassword   = doc["password"]     | "";
    String newWallet     = doc["walletId"]     | "";
    String newApiKey     = doc["apiKey"]       | "";
    int    newPulses     = doc["pulsesPerEur"] | PULSES_PER_EUR;

    if (newSSID.length() == 0 || newWallet.length() == 0 || newApiKey.length() == 0) {
      server.send(400, "application/json", "{\"error\":\"missing fields\"}");
      return;
    }

    // Basic UUID format check for wallet ID (must contain hyphens)
    if (newWallet.indexOf('-') < 0) {
      server.send(200, "application/json",
        "{\"success\":false,\"error\":\"Wallet ID should be a UUID — check you are using the BTC wallet ID\"}");
      return;
    }

    // Test WiFi connection before saving
    Serial.printf("Testing WiFi: %s\n", newSSID.c_str());
    WiFi.begin(newSSID.c_str(), newPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500); attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      server.send(200, "application/json",
        "{\"success\":false,\"error\":\"WiFi connection failed — check SSID and password\"}");
      return;
    }

    // Save all settings to NVS
    prefs.begin("satm", false);
    prefs.putString("ssid",         newSSID);
    prefs.putString("password",     newPassword);
    prefs.putString("walletId",     newWallet);
    prefs.putString("apiKey",       newApiKey);
    prefs.putInt("pulsesPerEur",    newPulses);
    prefs.end();

    server.send(200, "application/json", "{\"success\":true}");
    Serial.println("Settings saved — rebooting in 2s");
    delay(2000);
    ESP.restart();
  });

  // ── Factory reset ─────────────────────────────────────────
  // Clears ALL NVS keys including coin polarity — full re-detection on next boot
  server.on("/reset", HTTP_POST, []() {
    prefs.begin("satm", false);
    prefs.clear();
    prefs.end();
    server.send(200, "application/json", "{\"success\":true}");
    Serial.println("Factory reset — all NVS cleared");
    delay(1000);
    ESP.restart();
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
}
