#pragma once
// ═══════════════════════════════════════════════════════════════
//  ui.h  —  Tablet UI served by the ESP32 web server
//
//  Generated from coin_atm_esp32_new.html
//  Stored in PROGMEM (flash) — does not consume RAM at runtime
//
//  Usage in main.ino:
//    #include "ui.h"
//    server.on("/", HTTP_GET, []() {
//      server.send(200, "text/html", getUI());
//    });
// ═══════════════════════════════════════════════════════════════

#include <Arduino.h>
#include <pgmspace.h>

const char UI_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>⚡ Sats ATM</title>
<link href="https://fonts.googleapis.com/css2?family=Space+Mono:wght@400;700&family=Bebas+Neue&family=Nunito:wght@300;400;600;700;800;900&display=swap" rel="stylesheet">
<style>
:root {
  --white:   #FFFFFF;
  --black:   #0D0D0D;
  --yellow:  #FFD000;
  --orange:  #FF6B00;
  --gray:    #F2F2F2;
  --mid:     #AAAAAA;
  --border:  #E0E0E0;
  --mono:    'Space Mono', monospace;
  --display: 'Bebas Neue', sans-serif;
  --body:    'Nunito', sans-serif;
}

* { margin:0; padding:0; box-sizing:border-box; }

body {
  background: var(--white);
  color: var(--black);
  font-family: var(--body);
  height: 100vh;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

/* ── SCREENS ─────────────────────────────────────── */
.screen {
  position: absolute;
  inset: 0;
  display: flex;
  flex-direction: column;
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.4s ease;
}
.screen.active {
  opacity: 1;
  pointer-events: all;
}

/* ── TOP BAR ─────────────────────────────────────── */
.topbar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px 32px;
  border-bottom: 2px solid var(--black);
  flex-shrink: 0;
}
.logo {
  font-family: var(--display);
  font-size: 28px;
  letter-spacing: 0.08em;
  color: var(--black);
  display: flex;
  align-items: center;
  gap: 10px;
  cursor: default;
  user-select: none;
}
.logo-bolt {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 42px;
  height: 42px;
  background: var(--yellow);
  border-radius: 8px;
  position: relative;
  flex-shrink: 0;
}
.logo-coin {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 30px;
  height: 30px;
  background: #C8C8C8;
  border: 1.5px solid var(--black);
  border-radius: 50%;
  position: relative;
  overflow: visible;
}
.logo-coin svg {
  width: 28px;
  height: 28px;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  overflow: visible;
}
.gauge-wrap {
  display: flex;
  align-items: center;
  gap: 8px;
}
.gauge-label {
  font-family: var(--mono);
  font-size: 10px;
  letter-spacing: 0.15em;
  color: var(--mid);
}
.gauge-segments {
  display: flex;
  gap: 4px;
}
.seg {
  width: 22px;
  height: 10px;
  border-radius: 3px;
  transition: all 0.5s ease;
}
.seg.green  { background: #22c55e; box-shadow: 0 0 6px #22c55e88; }
.seg.amber  { background: var(--orange); box-shadow: 0 0 6px var(--orange); }
.seg.red    { background: #ef4444; box-shadow: 0 0 6px #ef444488; }
.seg.flash  { background: #ef4444; animation: flash-seg 0.8s ease-in-out infinite; }
.seg.empty  { background: var(--border); }
@keyframes flash-seg { 0%,100%{opacity:1} 50%{opacity:0.15} }

/* ── IDLE SCREEN ─────────────────────────────────── */
#screen-idle .body {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 0;
  padding: 40px 32px;
  position: relative;
}

.idle-label {
  font-family: var(--mono);
  font-size: 11px;
  letter-spacing: 0.2em;
  text-transform: uppercase;
  color: var(--mid);
  margin-bottom: 16px;
}

.idle-rate {
  font-family: var(--display);
  font-size: clamp(38px, 9vw, 88px);
  line-height: 1;
  color: var(--black);
  text-align: center;
  letter-spacing: 0.02em;
}
.idle-rate #idle-sats {
  color: var(--black);
}

.idle-rate-sub {
  font-family: var(--mono);
  font-size: 13px;
  color: var(--mid);
  margin-top: 14px;
  letter-spacing: 0.05em;
}

.insert-btn {
  margin-top: 52px;
  background: var(--yellow);
  color: var(--black);
  border: none;
  font-family: var(--display);
  font-size: 26px;
  letter-spacing: 0.1em;
  padding: 18px 56px;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.15s ease;
  border: 3px solid var(--black);
}
.insert-btn:hover {
  background: var(--orange);
  color: var(--white);
  transform: translateY(-2px);
  box-shadow: 4px 4px 0px var(--black);
}

/* Info button bottom right */
.info-btn {
  position: absolute;
  bottom: 28px;
  right: 32px;
  width: 38px;
  height: 38px;
  border-radius: 50%;
  border: 2px solid var(--border);
  background: var(--white);
  color: var(--mid);
  font-family: var(--mono);
  font-size: 15px;
  font-weight: 700;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;
}
.info-btn:hover {
  border-color: var(--black);
  color: var(--black);
}

/* ── COINS SCREEN ────────────────────────────────── */
#screen-coins .body,
#screen-sending .body,
#screen-success .body {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 40px 32px;
  gap: 32px;
}

.step-indicator {
  display: flex;
  align-items: center;
  gap: 0;
  margin-bottom: 8px;
}
.step-dot {
  width: 10px; height: 10px;
  border-radius: 50%;
  background: var(--border);
  transition: background 0.3s;
}
.step-dot.active { background: var(--orange); }
.step-dot.done   { background: var(--black); }
.step-line {
  width: 40px; height: 2px;
  background: var(--border);
  transition: background 0.3s;
}
.step-line.done { background: var(--black); }

.amount-block {
  text-align: center;
}
.amount-label {
  font-family: var(--mono);
  font-size: 11px;
  letter-spacing: 0.2em;
  text-transform: uppercase;
  color: var(--mid);
  margin-bottom: 10px;
}
.amount-eur {
  font-family: var(--display);
  font-size: clamp(60px, 14vw, 120px);
  line-height: 1;
  color: var(--black);
}
.amount-sats {
  font-family: var(--mono);
  font-size: 22px;
  color: var(--orange);
  margin-top: 8px;
}

.divider-line {
  width: 100%;
  max-width: 480px;
  height: 2px;
  background: var(--border);
}

.username-block {
  width: 100%;
  max-width: 480px;
  display: flex;
  flex-direction: column;
  gap: 10px;
}
.username-label-text {
  font-family: var(--mono);
  font-size: 11px;
  letter-spacing: 0.15em;
  text-transform: uppercase;
  color: var(--mid);
}
.username-field {
  display: flex;
  align-items: center;
  border: 3px solid var(--black);
  border-radius: 6px;
  overflow: hidden;
  background: var(--white);
  transition: border-color 0.2s;
}
.username-field:focus-within {
  border-color: var(--orange);
}
.username-input {
  flex: 1;
  border: none;
  outline: none;
  font-family: var(--mono);
  font-size: 24px;
  font-weight: 700;
  padding: 14px 16px;
  color: var(--black);
  background: transparent;
}
.username-input::placeholder {
  color: var(--border);
  font-weight: 400;
}
.username-at {
  font-family: var(--mono);
  font-size: 14px;
  color: var(--mid);
  padding: 14px 16px 14px 0;
  white-space: nowrap;
}

.send-btn {
  width: 100%;
  max-width: 480px;
  background: var(--black);
  color: var(--yellow);
  border: 3px solid var(--black);
  font-family: var(--display);
  font-size: 26px;
  letter-spacing: 0.12em;
  padding: 16px;
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.15s;
}
.send-btn:hover:not(:disabled) {
  background: var(--orange);
  border-color: var(--orange);
  color: var(--white);
  transform: translateY(-2px);
  box-shadow: 4px 4px 0 var(--black);
}
.send-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

/* Error */
.error-msg {
  font-family: var(--mono);
  font-size: 13px;
  color: var(--orange);
  text-align: center;
  max-width: 480px;
  display: none;
}
.error-msg.show { display: block; }

/* ── SENDING SCREEN ──────────────────────────────── */
.spinner {
  width: 56px; height: 56px;
  border: 4px solid var(--border);
  border-top-color: var(--orange);
  border-radius: 50%;
  animation: spin 0.9s linear infinite;
}
@keyframes spin { to { transform: rotate(360deg); } }

.sending-title {
  font-family: var(--display);
  font-size: 52px;
  letter-spacing: 0.05em;
  color: var(--black);
}
.sending-sub {
  font-family: var(--mono);
  font-size: 13px;
  color: var(--mid);
}

/* ── SUCCESS SCREEN ──────────────────────────────── */
.success-tick {
  font-size: 72px;
  animation: pop 0.5s cubic-bezier(0.34,1.56,0.64,1);
}
@keyframes pop { from{transform:scale(0)} to{transform:scale(1)} }

.success-title {
  font-family: var(--display);
  font-size: clamp(60px, 12vw, 100px);
  line-height: 1;
  color: var(--black);
  text-align: center;
}
.success-amount {
  font-family: var(--mono);
  font-size: 20px;
  color: var(--orange);
  text-align: center;
}
.success-address {
  font-family: var(--mono);
  font-size: 13px;
  color: var(--mid);
  text-align: center;
  margin-top: -4px;
}
  width: 100%;
  max-width: 400px;
  height: 6px;
  background: var(--border);
  border-radius: 3px;
  overflow: hidden;
  margin-top: 8px;
}
.success-bar-fill {
  height: 100%;
  background: var(--yellow);
  border-radius: 3px;
  width: 100%;
  transform-origin: left;
  animation: drain 4s linear forwards;
}
@keyframes drain { from{transform:scaleX(1)} to{transform:scaleX(0)} }

/* ── INFO OVERLAY ────────────────────────────────── */
.overlay {
  position: fixed;
  inset: 0;
  z-index: 100;
  background: rgba(13,13,13,0.7);
  display: flex;
  align-items: flex-end;
  justify-content: center;
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.3s;
}
.overlay.show {
  opacity: 1;
  pointer-events: all;
}
.overlay-panel {
  background: var(--white);
  border-top: 3px solid var(--black);
  width: 100%;
  max-width: 600px;
  padding: 32px;
  display: flex;
  flex-direction: column;
  gap: 20px;
  transform: translateY(30px);
  transition: transform 0.3s;
  border-radius: 16px 16px 0 0;
}
.overlay.show .overlay-panel {
  transform: translateY(0);
}
.overlay-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.overlay-title {
  font-family: var(--display);
  font-size: 32px;
  letter-spacing: 0.05em;
}
.overlay-close {
  width: 36px; height: 36px;
  border-radius: 50%;
  border: 2px solid var(--border);
  background: none;
  cursor: pointer;
  font-size: 18px;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;
}
.overlay-close:hover { border-color: var(--black); }

.info-steps {
  display: flex;
  flex-direction: column;
  gap: 12px;
}
.info-step {
  display: flex;
  align-items: flex-start;
  gap: 14px;
  padding: 14px;
  background: var(--gray);
  border-radius: 8px;
}
.info-step-num {
  width: 26px; height: 26px;
  border-radius: 4px;
  background: var(--yellow);
  display: flex; align-items: center; justify-content: center;
  font-family: var(--mono);
  font-size: 13px;
  font-weight: 700;
  flex-shrink: 0;
}
.info-step-text {
  font-size: 14px;
  color: var(--black);
  line-height: 1.5;
}
.info-step-text strong { font-weight: 600; }

.info-qr-row {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
  padding: 16px;
  border: 2px solid var(--border);
  border-radius: 8px;
}
.info-qr-box {
  width: 140px; height: 140px;
  background: var(--white);
  border-radius: 6px;
  flex-shrink: 0;
  display: flex; align-items: center; justify-content: center;
}
.info-qr-text strong {
  font-family: var(--display);
  font-size: 20px;
  display: block;
  text-align: center;
}

/* ── SETTINGS OVERLAY ────────────────────────────── */
.settings-overlay {
  position: fixed;
  inset: 0;
  z-index: 200;
  background: rgba(13,13,13,0.85);
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.3s;
}
.settings-overlay.show {
  opacity: 1;
  pointer-events: all;
}
.settings-panel {
  background: var(--white);
  border: 3px solid var(--black);
  border-radius: 12px;
  padding: 32px;
  width: 460px;
  max-width: 92vw;
  display: flex;
  flex-direction: column;
  gap: 20px;
  transform: translateY(20px);
  transition: transform 0.3s;
}
.settings-overlay.show .settings-panel {
  transform: translateY(0);
}
.settings-title {
  font-family: var(--display);
  font-size: 32px;
  letter-spacing: 0.05em;
}
.settings-sub {
  font-size: 12px;
  color: var(--mid);
  margin-top: -14px;
}
.form-section-label {
  font-family: var(--mono);
  font-size: 10px;
  text-transform: uppercase;
  letter-spacing: 0.15em;
  color: var(--mid);
  border-bottom: 1px solid var(--border);
  padding-bottom: 6px;
  margin-bottom: 4px;
}
.form-row { display: flex; flex-direction: column; gap: 4px; }
.form-label { font-size: 12px; color: var(--mid); }
.form-input {
  background: var(--gray);
  border: 2px solid var(--border);
  border-radius: 6px;
  padding: 9px 12px;
  color: var(--black);
  font-family: var(--mono);
  font-size: 13px;
  width: 100%;
  outline: none;
  transition: border-color 0.2s;
}
.form-input:focus { border-color: var(--black); }
.form-input::placeholder { color: var(--mid); }
.pw-wrap { position: relative; }
.pw-wrap .form-input { padding-right: 42px; }
.pw-toggle {
  position: absolute; right: 10px; top: 50%; transform: translateY(-50%);
  background: none; border: none; color: var(--mid); cursor: pointer; font-size: 13px;
}
.settings-status {
  font-family: var(--mono);
  font-size: 12px;
  padding: 8px 12px;
  border-radius: 6px;
  background: var(--gray);
  border: 1px solid var(--border);
  color: var(--mid);
  min-height: 34px;
  display: flex;
  align-items: center;
}
.settings-status.ok   { border-color: #22c55e; color: #16a34a; background: #f0fdf4; }
.settings-status.fail { border-color: var(--orange); color: var(--orange); background: #fff7f0; }
.settings-status.busy { border-color: var(--yellow); color: #92400e; background: #fffbeb; }
.btn-row { display: flex; gap: 8px; justify-content: flex-end; }
.btn {
  font-family: var(--body);
  font-size: 13px;
  font-weight: 600;
  padding: 8px 18px;
  border-radius: 6px;
  border: 2px solid var(--border);
  background: var(--white);
  color: var(--black);
  cursor: pointer;
  transition: all 0.15s;
}
.btn:hover { border-color: var(--black); }
.btn:disabled { opacity: 0.4; cursor: not-allowed; }
.btn.primary {
  background: var(--black);
  border-color: var(--black);
  color: var(--yellow);
}
.btn.primary:hover { background: var(--orange); border-color: var(--orange); color: var(--white); }
.btn.danger { border-color: rgba(239,68,68,0.3); color: #ef4444; }
.btn.danger:hover { background: #fff5f5; border-color: #ef4444; }
.btn.sm { padding: 6px 12px; font-size: 12px; }
.long-press-hint { font-size: 10px; color: var(--mid); text-align: center; margin-top: -8px; }
</style>
</head>
<body>

<!-- ══ SCREEN: IDLE ══════════════════════════════════ -->
<div class="screen active" id="screen-idle">
  <div class="topbar">
    <div class="logo" id="logo-hold">
      <span class="logo-bolt"><span class="logo-coin"><svg viewBox="0 0 24 36" fill="none" xmlns="http://www.w3.org/2000/svg"><polygon points="14,0 4,20 12,20 10,36 20,16 12,16" fill="var(--yellow)" stroke="var(--black)" stroke-width="1.2" stroke-linejoin="round"/></svg></span></span>
      SATS ATM
    </div>
    <div class="gauge-wrap">
      <span class="gauge-label">WALLET</span>
      <div class="gauge-segments">
        <div class="seg empty" id="seg1"></div>
        <div class="seg empty" id="seg2"></div>
        <div class="seg empty" id="seg3"></div>
      </div>
    </div>
  </div>
  <div class="body">
    <div class="idle-label">Exchange Rate</div>
    <div class="idle-rate" id="idle-rate-display">
      €1 = <span id="idle-sats">—</span> SATS
    </div>
    <button class="insert-btn" onclick="showScreen('screen-coins')">INSERT COINS</button>
    <button class="info-btn" onclick="openInfo()" title="How it works">?</button>
  </div>
</div>

<!-- ══ SCREEN: COINS / USERNAME ══════════════════════ -->
<div class="screen" id="screen-coins">
  <div class="topbar">
    <div class="logo">
      <span class="logo-bolt"><span class="logo-coin"><svg viewBox="0 0 24 36" fill="none" xmlns="http://www.w3.org/2000/svg"><polygon points="14,0 4,20 12,20 10,36 20,16 12,16" fill="var(--yellow)" stroke="var(--black)" stroke-width="1.2" stroke-linejoin="round"/></svg></span></span>
      SATS ATM
    </div>
    <div class="gauge-wrap">
      <span class="gauge-label">WALLET</span>
      <div class="gauge-segments">
        <div class="seg empty" id="seg1b"></div>
        <div class="seg empty" id="seg2b"></div>
        <div class="seg empty" id="seg3b"></div>
      </div>
    </div>
  </div>
  <div class="body">
    <div class="step-indicator">
      <div class="step-dot done" id="sd1"></div>
      <div class="step-line done" id="sl1"></div>
      <div class="step-dot active" id="sd2"></div>
      <div class="step-line" id="sl2"></div>
      <div class="step-dot" id="sd3"></div>
    </div>

    <div class="amount-block">
      <div class="amount-label">You inserted</div>
      <div class="amount-eur" id="amount-eur">€0.00</div>
      <div class="amount-sats" id="amount-sats">0 sats</div>
    </div>

    <div class="divider-line"></div>

    <div class="username-block">
      <div class="username-label-text">Enter Blink username</div>
      <div class="username-field">
        <input
          class="username-input"
          id="username-input"
          type="text"
          placeholder="yourname"
          autocomplete="off"
          autocorrect="off"
          autocapitalize="none"
          spellcheck="false"
          maxlength="30"
        />
        <span class="username-at">@blink.sv</span>
      </div>
      <div class="error-msg" id="error-msg">⚠ Check your username and try again</div>
    </div>

    <button class="send-btn" id="send-btn" onclick="sendSats()">SEND MY SATS ⚡</button>
  </div>
</div>

<!-- ══ SCREEN: SENDING ════════════════════════════════ -->
<div class="screen" id="screen-sending">
  <div class="topbar">
    <div class="logo">
      <span class="logo-bolt"><span class="logo-coin"><svg viewBox="0 0 24 36" fill="none" xmlns="http://www.w3.org/2000/svg"><polygon points="14,0 4,20 12,20 10,36 20,16 12,16" fill="var(--yellow)" stroke="var(--black)" stroke-width="1.2" stroke-linejoin="round"/></svg></span></span>
      SATS ATM
    </div>
    <div class="gauge-wrap">
      <span class="gauge-label">WALLET</span>
      <div class="gauge-segments">
        <div class="seg empty" id="seg1c"></div>
        <div class="seg empty" id="seg2c"></div>
        <div class="seg empty" id="seg3c"></div>
      </div>
    </div>
  </div>
  <div class="body">
    <div class="step-indicator">
      <div class="step-dot done"></div>
      <div class="step-line done"></div>
      <div class="step-dot done"></div>
      <div class="step-line done"></div>
      <div class="step-dot active"></div>
    </div>
    <div class="spinner"></div>
    <div class="sending-title">SENDING…</div>
    <div class="sending-sub">Please wait</div>
  </div>
</div>

<!-- ══ SCREEN: SUCCESS ════════════════════════════════ -->
<div class="screen" id="screen-success">
  <div class="topbar">
    <div class="logo">
      <span class="logo-bolt"><span class="logo-coin"><svg viewBox="0 0 24 36" fill="none" xmlns="http://www.w3.org/2000/svg"><polygon points="14,0 4,20 12,20 10,36 20,16 12,16" fill="var(--yellow)" stroke="var(--black)" stroke-width="1.2" stroke-linejoin="round"/></svg></span></span>
      SATS ATM
    </div>
    <div class="gauge-wrap">
      <span class="gauge-label">WALLET</span>
      <div class="gauge-segments">
        <div class="seg empty" id="seg1d"></div>
        <div class="seg empty" id="seg2d"></div>
        <div class="seg empty" id="seg3d"></div>
      </div>
    </div>
  </div>
  <div class="body">
    <div class="step-indicator">
      <div class="step-dot done"></div>
      <div class="step-line done"></div>
      <div class="step-dot done"></div>
      <div class="step-line done"></div>
      <div class="step-dot done"></div>
    </div>
    <div class="success-tick">✅</div>
    <div class="success-title">SATS<br>SENT</div>
    <div class="success-amount" id="success-amount">0 sats sent</div>
    <div class="success-bar"><div class="success-bar-fill" id="success-bar-fill"></div></div>
    <div class="success-address" id="success-address"></div>
  </div>
</div>

<!-- ══ INFO OVERLAY ═══════════════════════════════════ -->
<div class="overlay" id="info-overlay" onclick="closeInfoOutside(event)">
  <div class="overlay-panel">
    <div class="overlay-header">
      <div class="overlay-title">HOW IT WORKS</div>
      <button class="overlay-close" onclick="closeInfo()">✕</button>
    </div>
    <div class="info-steps">
      <div class="info-step">
        <div class="info-step-num">1</div>
        <div class="info-step-text"><strong>Insert coins</strong> — up to €10 per session</div>
      </div>
      <div class="info-step">
        <div class="info-step-num">2</div>
        <div class="info-step-text"><strong>Enter your Blink username</strong> — the one you set in the app</div>
      </div>
      <div class="info-step">
        <div class="info-step-num">3</div>
        <div class="info-step-text"><strong>Receive sats instantly</strong> — sent directly to your Blink wallet</div>
      </div>
    </div>
    <div class="info-qr-row">
      <div class="info-qr-text"><strong>BLINK WALLET APP</strong></div>
      <div class="info-qr-box" id="info-qr"></div>
    </div>
  </div>
</div>

<!-- ══ SETTINGS OVERLAY ═══════════════════════════════ -->
<div class="settings-overlay" id="settings-overlay">
  <div class="settings-panel">
    <div>
      <div class="settings-title">⚙ SETTINGS</div>
      <div class="settings-sub">Saved to ESP32 flash — not visible to users</div>
    </div>
    <div>
      <div class="form-section-label">📶 WiFi</div>
      <div style="display:flex;flex-direction:column;gap:10px;margin-top:10px">
        <div class="form-row">
          <label class="form-label">Network name (SSID)</label>
          <input class="form-input" id="f-ssid" type="text" placeholder="e.g. MyPhone Hotspot" autocomplete="off" spellcheck="false">
        </div>
        <div class="form-row">
          <label class="form-label">Password</label>
          <div class="pw-wrap">
            <input class="form-input" id="f-password" type="password" placeholder="WiFi password" autocomplete="off">
            <button class="pw-toggle" onclick="togglePw('f-password',this)">👁</button>
          </div>
        </div>
      </div>
    </div>
    <div>
      <div class="form-section-label">⚡ Blink Wallet</div>
      <div style="display:flex;flex-direction:column;gap:10px;margin-top:10px">
        <div class="form-row">
          <label class="form-label">Wallet ID</label>
          <input class="form-input" id="f-walletid" type="text" placeholder="Your Blink wallet ID" autocomplete="off" spellcheck="false">
        </div>
        <div class="form-row">
          <label class="form-label">API Key</label>
          <div class="pw-wrap">
            <input class="form-input" id="f-apikey" type="password" placeholder="Your Blink API key" autocomplete="off">
            <button class="pw-toggle" onclick="togglePw('f-apikey',this)">👁</button>
          </div>
        </div>
      </div>
    </div>
    <div class="settings-status" id="settings-status">Enter credentials above to get started</div>
    <div class="btn-row">
      <button class="btn danger sm" onclick="factoryReset()">Factory Reset</button>
      <button class="btn sm" id="close-btn" onclick="closeSettings()" style="display:none">Close</button>
      <button class="btn primary" id="save-btn" onclick="saveSettings()">Save & Connect</button>
    </div>
    <div class="long-press-hint">Hold the ⚡ logo 3 seconds + PIN to access settings</div>
  </div>
</div>

<!-- ══ QR LIBRARY ═════════════════════════════════════ -->
<script>
var QRCode;!function(){function a(a){this.mode=c.MODE_8BIT_BYTE,this.data=a,this.parsedData=[];for(var b=[],d=0,e=this.data.length;e>d;d++){var f=this.data.charCodeAt(d);if(f>65536)b[0]=240|(1835008&f)>>>18,b[1]=128|(258048&f)>>>12,b[2]=128|(4032&f)>>>6,b[3]=128|63&f;else if(f>2048)b[0]=224|(61440&f)>>>12,b[1]=128|(4032&f)>>>6,b[2]=128|63&f;else if(f>128)b[0]=192|(1984&f)>>>6,b[1]=128|63&f;else b[0]=f;this.parsedData=this.parsedData.concat(b)}this.parsedData.length!=this.data.length&&(this.parsedData.unshift(191),this.parsedData.unshift(187),this.parsedData.unshift(239))}function b(a,b){this.typeNumber=a,this.errorCorrectLevel=b,this.modules=null,this.moduleCount=0,this.dataCache=null,this.dataList=[]}function i(a,b){if(void 0==a.length)throw new Error(a.length);for(var c=0,d=0;d<a.length;d++)c+=a[d]<<8*(a.length-d-1);this.num=c}function j(a){this.gexp=[],this.glog=[];for(var b=1,c=0;256>c;c++)this.gexp[c]=b,b<<=1,b>=256&&(b^=285);for(var c=0;255>c;c++)this.glog[this.gexp[c]]=c;for(var d=[],e=0;e<a;e++)d[e]=0;d[0]=1;for(var e=0;a>e;e++){for(var f=[],g=0;g<d.length+1;g++)f[g]=0;for(var g=0;g<d.length;g++)for(var h=0;h<2;h++)f[g+h]^=this.gexp[(this.glog[d[g]]+e)%255];d=f}this.num=d}function k(a,b){for(var c=b.num.length,d=[],e=0;e<a.num.length-b.num.length+1;e++){var f=this.glog[a.num[e]];a.num[e]=0;for(var g=0;g<b.num.length;g++)a.num[e+g]^=this.gexp[(f+this.glog[b.num[g]])%255]}return new j(0)}function n(a,b){var c=-1;switch(b){case 1:c=[],c[1]=1,c[7]=26,c[8]=19,c[13]=25;break;case 0:c=[],c[1]=0,c[7]=20,c[8]=10,c[13]=17}return c[a]}a.prototype={getLength:function(){return this.parsedData.length},write:function(a){for(var b=0,c=this.parsedData.length;c>b;b++)a.put(this.parsedData[b],8)}};var c={MODE_NUMBER:1,MODE_ALPHA_NUM:2,MODE_8BIT_BYTE:4,MODE_KANJI:8},d={L:1,M:0,Q:3,H:2},e={PATTERN000:0,PATTERN001:1,PATTERN010:2,PATTERN011:3,PATTERN100:4,PATTERN101:5,PATTERN110:6,PATTERN111:7};b.prototype={addData:function(b){var c=new a(b);this.dataList.push(c),this.dataCache=null},isDark:function(a,b){if(0>a||this.moduleCount<=a||0>b||this.moduleCount<=b)throw new Error(a+","+b);return this.modules[a][b]},getModuleCount:function(){return this.moduleCount},make:function(){this.makeImpl(!1,this.getBestMaskPattern())},makeImpl:function(a,c){this.moduleCount=4*this.typeNumber+17,this.modules=function(a){var b=[];for(var c=0;a>c;c++){b[c]=[];for(var d=0;a>d;d++)b[c][d]=null}return b}(this.moduleCount),this.setupPositionProbePattern(0,0),this.setupPositionProbePattern(this.moduleCount-7,0),this.setupPositionProbePattern(0,this.moduleCount-7),this.setupPositionAdjustPattern(),this.setupTimingPattern(),this.setupTypeInfo(a,c),this.typeNumber>=7&&this.setupTypeNumber(a),null==this.dataCache&&(this.dataCache=b.createData(this.typeNumber,this.errorCorrectLevel,this.dataList)),this.mapData(this.dataCache,c)},setupPositionProbePattern:function(a,b){for(var c=-1;7>=c;c++)if(!(-1>=a+c||this.moduleCount<=a+c))for(var d=-1;7>=d;d++)-1>=b+d||this.moduleCount<=b+d||(this.modules[a+c][b+d]=c>=0&&6>=c&&(0==d||6==d)||d>=0&&6>=d&&(0==c||6==c)||c>=2&&4>=c&&d>=2&&4>=d?!0:!1)},getBestMaskPattern:function(){for(var a=0,b=0,c=0;8>c;c++){this.makeImpl(!0,c);var d=f.getLostPoint(this);(0==c||a>d)&&(a=d,b=c)}return b},setupTimingPattern:function(){for(var a=8;this.moduleCount-8>a;a++)null==this.modules[a][6]&&(this.modules[a][6]=0==a%2);for(var b=8;this.moduleCount-8>b;b++)null==this.modules[6][b]&&(this.modules[6][b]=0==b%2)},setupPositionAdjustPattern:function(){for(var a=g.getPatternPosition(this.typeNumber),b=0;b<a.length;b++)for(var c=0;c<a.length;c++){var d=a[b],e=a[c];if(null==this.modules[d][e])for(var f=-2;2>=f;f++)for(var h=-2;2>=h;h++)this.modules[d+f][e+h]=-2==f||2==f||-2==h||2==h||0==f&&0==h?!0:!1}},setupTypeNumber:function(a){for(var b=h.getBCHTypeNumber(this.typeNumber),c=0;18>c;c++){var d=!a&&1==(b>>c&1);this.modules[Math.floor(c/3)][c%3+this.moduleCount-8-3]=d}for(var c=0;18>c;c++){var d=!a&&1==(b>>c&1);this.modules[c%3+this.moduleCount-8-3][Math.floor(c/3)]=d}},setupTypeInfo:function(a,b){for(var c=d[this.errorCorrectLevel]<<3|b,e=h.getBCHTypeInfo(c),f=0;15>f;f++){var g=!a&&1==(e>>f&1);6>f?this.modules[f][8]=g:7>f?this.modules[f+1][8]=g:this.modules[this.moduleCount-15+f][8]=g}for(var f=0;15>f;f++){var g=!a&&1==(e>>f&1);8>f?this.modules[8][this.moduleCount-f-1]=g:9>f?this.modules[8][15-f-1+1]=g:this.modules[8][15-f-1]=g}this.modules[this.moduleCount-8][8]=!a},mapData:function(a,b){for(var c=-1,d=this.moduleCount-1,e=7,g=0,h=this.moduleCount-1;h>0;h-=2)for(6==h&&h--;;){for(var i=0;2>i;i++)if(null==this.modules[d][h-i]){var j=!1;g<a.length&&(j=1==(a[g]>>>e&1)),f.getMask(b,d,h-i)&&(j=!j),this.modules[d][h-i]=j,e--,-1==e&&(g++,e=7)}if(d+=c,0>d||this.moduleCount<=d){d-=c,c=-c;break}}}};var f={getBCHDigit:function(a){for(var b=0;0!=a;)b++,a>>>=1;return b},getBCHTypeInfo:function(a){for(var b=a<<10;f.getBCHDigit(b)-f.getBCHDigit(1335)>=0;)b^=1335<<f.getBCHDigit(b)-f.getBCHDigit(1335);return 21522^(a<<10|b)},getBCHTypeNumber:function(a){for(var b=a<<12;f.getBCHDigit(b)-f.getBCHDigit(7973)>=0;)b^=7973<<f.getBCHDigit(b)-f.getBCHDigit(7973);return a<<12|b},getPatternPosition:function(a){return n(a,1)},getMask:function(a,b,c){switch(a){case e.PATTERN000:return 0==(b+c)%2;case e.PATTERN001:return 0==b%2;case e.PATTERN010:return 0==c%3;case e.PATTERN011:return 0==(b+c)%3;case e.PATTERN100:return 0==(Math.floor(b/2)+Math.floor(c/3))%2;case e.PATTERN101:return 0==b*c%2+b*c%3;case e.PATTERN110:return 0==(b*c%2+b*c%3)%2;case e.PATTERN111:return 0==(b*c%3+(b+c)%2)%2;default:throw new Error("bad maskPattern:"+a)}},getLostPoint:function(a){for(var b=a.getModuleCount(),c=0,d=0;b>d;d++)for(var e=0;b>e;e++){for(var g=0,h=a.isDark(d,e),i=-1;1>=i;i++)if(!(0>d+i||b<=d+i))for(var j=-1;1>=j;j++)0>e+j||b<=e+j||(d+i!=d||e+j!=e)&&h==a.isDark(d+i,e+j)&&g++;g>5&&(c+=3+g-5)}for(var d=0;b-1>d;d++)for(var e=0;b-1>e;e++){var k=0;a.isDark(d,e)&&k++,a.isDark(d+1,e)&&k++,a.isDark(d,e+1)&&k++,a.isDark(d+1,e+1)&&k++,(0==k||4==k)&&(c+=3)}for(var d=0;b>d;d++)for(var e=0;b-6>e;e++)a.isDark(d,e)&&!a.isDark(d,e+1)&&a.isDark(d,e+2)&&a.isDark(d,e+3)&&a.isDark(d,e+4)&&!a.isDark(d,e+5)&&a.isDark(d,e+6)&&(c+=40);for(var e=0;b>e;e++)for(var d=0;b-6>d;d++)a.isDark(d,e)&&!a.isDark(d+1,e)&&a.isDark(d+2,e)&&a.isDark(d+3,e)&&a.isDark(d+4,e)&&!a.isDark(d+5,e)&&a.isDark(d+6,e)&&(c+=40);for(var l=0,d=0;b>d;d++)for(var e=0;b>e;e++)a.isDark(d,e)&&l++;var m=Math.abs(100*l/b/b-50)/5;return c+=10*m},createData:function(a,b,c){for(var e=p.getRSBlocks(a,b),f=new o,g=0;g<c.length;g++){var h=c[g];f.put(h.mode,4),f.put(h.getLength(),q.getLengthInBits(h.mode,a)),h.write(f)}for(var i=0,g=0;g<e.length;g++)i+=e[g].dataCount;if(f.getLengthInBits()>8*i)throw new Error("code length overflow. ("+f.getLengthInBits()+">"+8*i+")");for(f.getLengthInBits()+4<=8*i&&f.put(0,4);0!=f.getLengthInBits()%8;)f.putBit(!1);for(;;){if(f.getLengthInBits()>=8*i)break;if(f.put(236,8),f.getLengthInBits()>=8*i)break;f.put(17,8)}return b.createBytes(f,e)},createBytes:function(a,b){for(var c=0,d=0,e=0,f=new Array(b.length),g=new Array(b.length),h=0;h<b.length;h++){var i=b[h].dataCount,l=b[h].totalCount-i;d=Math.max(d,i),e=Math.max(e,l),f[h]=new Array(i);for(var m=0;m<f[h].length;m++)f[h][m]=255&a.buffer[c++];g[h]=new Array(l);for(var m=0;m<g[h].length;m++)g[h][m]=0}var n=new Array(e),k=0;for(var h=0;h<b.length;h++)for(var m=0;m<b[h].totalCount-b[h].dataCount;m++)g[h][m]=0;var o=[],m=0;for(;d>m;m++)for(var h=0;h<b.length;h++)m<f[h].length&&(o[k++]=f[h][m]);for(var m=0;e>m;m++)for(var h=0;h<b.length;h++)m<g[h].length&&(o[k++]=g[h][m]);return o}};i.prototype={get:function(a){return 1==(this.num>>>a&1)},getLengthInBits:function(){return this.num},put:function(a,b){for(var c=0;b>c;c++)this.putBit(this.get(b-c-1),a)},putBit:function(a){var b=Math.floor(this.num/8);this.buffer.length<=b&&this.buffer.push(0),a&&(this.buffer[b]|=128>>>this.num%8),this.num++}};var g={getPatternPosition:function(a){return n(a,0)}};var h={getBCHTypeInfo:function(a){return f.getBCHTypeInfo(a)},getBCHTypeNumber:function(a){return f.getBCHTypeNumber(a)}};var o=function(){this.buffer=[],this.num=0};o.prototype={get:function(a){var b=Math.floor(a/8);return 1==(this.buffer[b]>>>7-a%8&1)},put:function(a,b){for(var c=0;b>c;c++)this.putBit(1==(a>>>b-c-1&1))},getLengthInBits:function(){return this.num},putBit:function(a){var b=Math.floor(this.num/8);this.buffer.length<=b&&this.buffer.push(0),a&&(this.buffer[b]|=128>>>this.num%8),this.num++}};var p={getRSBlocks:function(a,b){var c=p.getRSBlockTable(a,b);if(void 0==c)throw new Error("bad rs block @ typeNumber:"+a+"/errorCorrectLevel:"+b);for(var d=c.length/3,e=[],f=0;d>f;f++)for(var g=c[3*f+0],h=c[3*f+1],i=c[3*f+2],j=0;g>j;j++)e.push(new q(h,i));return e},getRSBlockTable:function(a,b){switch(b){case d.L:return n(a,0);case d.M:return n(a,1);case d.Q:return n(a,2);case d.H:return n(a,3)}}};var q=function(a,b){this.totalCount=a,this.dataCount=b};q.getLengthInBits=function(a,b){switch(a){case c.MODE_NUMBER:return 10>b?10:27>b?12:14;case c.MODE_ALPHA_NUM:return 10>b?9:27>b?11:13;case c.MODE_8BIT_BYTE:return 10>b?8:16;case c.MODE_KANJI:return 10>b?8:27>b?10:12;default:throw new Error("mode:"+a)}};var r=function(a,b,c){this.el=a,this.htOption={width:256,height:256,typeNumber:4,colorDark:"#000000",colorLight:"#ffffff",correctLevel:d.H};if(b){"string"==typeof b&&(b={text:b});for(var e in b)this.htOption[e]=b[e]}if("string"==typeof a&&(this.el=document.getElementById(a)),this.htOption.useSVG)this.oQRCode=null;else{this.oQRCode=new b(this.htOption.typeNumber,this.htOption.correctLevel),this.oQRCode.addData(this.htOption.text),this.oQRCode.make()}c?this._oDrawing=new c(this.el,this.htOption):this._oDrawing=new s(this.el,this.htOption),this._oDrawing.draw(this.oQRCode)};r.prototype.makeCode=function(a){this.oQRCode.addData(a),this.oQRCode.make(),this._oDrawing.draw(this.oQRCode)};var s=function(a,b){this._bIsPainted=!1,this._htOption=b;var c=document.createElement("canvas");c.width=b.width,c.height=b.height,a.appendChild(c),this._elCanvas=c,this._oContext=c.getContext("2d"),this._oContext.fillStyle=b.colorLight,this._oContext.fillRect(0,0,b.width,b.height)};s.prototype.draw=function(a){var b=this._htOption,c=this._oContext,d=a.getModuleCount(),e=Math.floor(b.width/d),f=Math.floor(b.height/d);c.fillStyle=b.colorLight,c.fillRect(0,0,b.width,b.height);for(var g=0;d>g;g++)for(var h=0;d>h;h++){var i=a.isDark(g,h);c.fillStyle=i?b.colorDark:b.colorLight,c.fillRect(h*e,g*f,e,f)}this._bIsPainted=!0},s.prototype.clear=function(){this._oContext.clearRect(0,0,this._elCanvas.width,this._elCanvas.height),this._bIsPainted=!1};QRCode=r}();
</script>

<!-- ══ APP LOGIC ═══════════════════════════════════════ -->
<script>
const POLL_MS  = 2000;
const SETTINGS_PIN = '1928';

let lastState     = '';
let settingsOpen  = false;
let infoOpen      = false;
let currentSats   = 0;
let currentCredit = 0;
let paidShown     = false;

// Generate QR on load
new QRCode(document.getElementById('info-qr'), {
  text: 'https://blink.sv',
  width: 130, height: 130,
  colorDark: '#0D0D0D', colorLight: '#ffffff',
  correctLevel: QRCode.CorrectLevel.M
});

// ── Screen transitions ────────────────────────────────
function showScreen(id) {
  document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
  document.getElementById(id).classList.add('active');
}

// ── Long-press logo 3s + PIN → settings ──────────────
(function(){
  const el = document.getElementById('logo-hold');
  let t = null;
  function start(){
    t = setTimeout(() => {
      const pin = prompt('Enter PIN:');
      if (pin === SETTINGS_PIN) openSettings(true);
    }, 3000);
  }
  function cancel(){ clearTimeout(t); }
  el.addEventListener('mousedown',  start);
  el.addEventListener('touchstart', start, {passive:true});
  el.addEventListener('mouseup',    cancel);
  el.addEventListener('mouseleave', cancel);
  el.addEventListener('touchend',   cancel);
})();

// ── Info overlay ──────────────────────────────────────
function openInfo() {
  infoOpen = true;
  document.getElementById('info-overlay').classList.add('show');
}
function closeInfo() {
  infoOpen = false;
  document.getElementById('info-overlay').classList.remove('show');
}
function closeInfoOutside(e) {
  if (e.target === document.getElementById('info-overlay')) closeInfo();
}

// ── Settings ──────────────────────────────────────────
function openSettings(canClose) {
  settingsOpen = true;
  document.getElementById('settings-overlay').classList.add('show');
  document.getElementById('close-btn').style.display = canClose ? 'inline-block' : 'none';
}
function closeSettings() {
  document.getElementById('settings-overlay').classList.remove('show');
  settingsOpen = false;
}
function togglePw(id, btn) {
  const el = document.getElementById(id);
  el.type = el.type === 'password' ? 'text' : 'password';
  btn.textContent = el.type === 'password' ? '👁' : '🙈';
}
async function saveSettings() {
  const ssid     = document.getElementById('f-ssid').value.trim();
  const password = document.getElementById('f-password').value;
  const walletId = document.getElementById('f-walletid').value.trim();
  const apiKey   = document.getElementById('f-apikey').value.trim();
  if (!ssid || !walletId || !apiKey)
    return setStatus('fail', 'SSID, Wallet ID and API Key are required');
  const btn = document.getElementById('save-btn');
  btn.disabled = true;
  setStatus('busy', 'Testing WiFi connection…');
  try {
    const r = await fetch('/settings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid, password, walletId, apiKey })
    });
    const d = await r.json();
    if (d.success) {
      setStatus('ok', 'Saved! ESP32 rebooting…');
      setTimeout(() => location.reload(), 4000);
    } else {
      setStatus('fail', d.error || 'Connection failed — check credentials');
      btn.disabled = false;
    }
  } catch(e) {
    setStatus('fail', 'Cannot reach ESP32');
    btn.disabled = false;
  }
}
async function factoryReset() {
  if (!confirm('Clear all saved settings and reboot?')) return;
  try {
    await fetch('/reset', { method: 'POST' });
    setStatus('ok', 'Reset complete — rebooting…');
  } catch(e) { setStatus('fail', 'Cannot reach ESP32'); }
}
function setStatus(type, msg) {
  const el = document.getElementById('settings-status');
  el.className = 'settings-status' + (type ? ' '+type : '');
  el.textContent = msg;
}

// ── Send sats ─────────────────────────────────────────
async function sendSats() {
  const username = document.getElementById('username-input').value.trim().toLowerCase();
  const errEl = document.getElementById('error-msg');
  errEl.classList.remove('show');

  if (!username) {
    document.getElementById('username-input').focus();
    return;
  }
  if (username.includes('@') || username.includes(' ')) {
    errEl.textContent = '⚠ Just your username — no @blink.sv needed';
    errEl.classList.add('show');
    return;
  }

  const lnAddress = username + '@blink.sv';
  showScreen('screen-sending');

  try {
    const r = await fetch('/send', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ lnAddress, sats: currentSats })
    });
    const d = await r.json();
    if (d.success) {
      showSuccess(lnAddress);
    } else {
      showScreen('screen-coins');
      errEl.textContent = '⚠ ' + (d.error || 'Payment failed — try again');
      errEl.classList.add('show');
    }
  } catch(e) {
    showScreen('screen-coins');
    errEl.textContent = '⚠ Cannot reach ATM — check connection';
    errEl.classList.add('show');
  }
}

document.getElementById('username-input').addEventListener('keydown', function(e) {
  if (e.key === 'Enter') sendSats();
});

// ── Success ───────────────────────────────────────────
function showSuccess(lnAddress) {
  document.getElementById('success-amount').textContent =
    parseInt(currentSats).toLocaleString() + ' sats sent';
  document.getElementById('success-address').textContent =
    lnAddress ? 'to ' + lnAddress : '';
  // Restart the bar animation
  const bar = document.getElementById('success-bar-fill');
  bar.style.animation = 'none';
  bar.offsetHeight; // reflow
  bar.style.animation = 'drain 4s linear forwards';
  showScreen('screen-success');
  setTimeout(resetToIdle, 4500);
}

function resetToIdle() {
  paidShown     = false;
  currentSats   = 0;
  currentCredit = 0;
  document.getElementById('username-input').value = '';
  document.getElementById('error-msg').classList.remove('show');
  showScreen('screen-idle');
}

// ── Gauge ─────────────────────────────────────────────
function setGauge(level) {
  ['','b','c','d'].forEach(suffix => {
    const s = [1,2,3].map(n => document.getElementById('seg'+n+suffix));
    if (!s[0]) return;
    s.forEach(el => el.className = 'seg empty');
    if      (level==='high')     { s[0].className='seg green'; s[1].className='seg green'; s[2].className='seg green'; }
    else if (level==='mid')      { s[0].className='seg amber'; s[1].className='seg amber'; }
    else if (level==='low')      { s[0].className='seg red'; }
    else if (level==='critical') { s[0].className='seg flash'; }
  });
}

// ── Poll /state every 2s ──────────────────────────────
async function pollState() {
  if (settingsOpen) return;
  try {
    const r = await fetch('/state');
    if (!r.ok) return;
    const d = await r.json();

    // First boot — no credentials
    if (d.needsSetup) { if (!settingsOpen) openSettings(false); return; }

    // Update rate display
    if (d.rate > 0) {
      document.getElementById('idle-sats').textContent = parseInt(d.rate).toLocaleString();
    }

    // Gauge
    if (d.gauge) setGauge(d.gauge);

    // State transitions
    if (d.state === 'READY' && d.sats > 0) {
      currentSats   = d.sats;
      currentCredit = d.credit;
      document.getElementById('amount-eur').textContent  = '€' + parseFloat(d.credit).toFixed(2);
      document.getElementById('amount-sats').textContent = parseInt(d.sats).toLocaleString() + ' sats';
      if (lastState !== 'READY') {
        showScreen('screen-coins');
        setTimeout(() => document.getElementById('username-input').focus(), 200);
      }
    } else if (d.state === 'IDLE' && lastState && lastState !== 'IDLE' && !paidShown) {
      resetToIdle();
    }

    lastState = d.state;
  } catch(e) { /* hold last state */ }
}

pollState();
setInterval(pollState, POLL_MS);
</script>

</body>
</html>
)rawhtml";

// Returns UI HTML from PROGMEM as an Arduino String
String getUI() {
  return String(FPSTR(UI_HTML));
}
