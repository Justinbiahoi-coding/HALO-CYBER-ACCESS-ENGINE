#include <iostream>
#include <cstdlib> 
#include "HaloEngine.h"
#include "httplib.h"

using namespace std;
using namespace httplib;

static const char* DASHBOARD_HTML = R"HALOHTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Halo Cyber Access Engine</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Rajdhani:wght@400;500;600;700&display=swap" rel="stylesheet">
<style>
/* ── Reset & CSS Variables ─────────────────────────────────────────────── */
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
:root {
  --bg:        #0D0F14;
  --bg2:       #13161F;
  --bg3:       #0A0C10;
  --bg4:       #1A1E2A;
  --border:    #1E2438;
  --border2:   #252A3A;
  --accent:    #00C8FF;
  --accent2:   #0099CC;
  --accent-dim: rgba(0,200,255,0.12);
  --green:     #00E87A;
  --green-dim: rgba(0,232,122,0.12);
  --orange:    #FF8C00;
  --orange-dim:rgba(255,140,0,0.12);
  --red:       #FF3C5A;
  --red-dim:   rgba(255,60,90,0.12);
  --yellow:    #FFD700;
  --text:      #B8BDD4;
  --text2:     #8B90A8;
  --white:     #E8EAF0;
  --font-mono: 'Share Tech Mono', 'Courier New', monospace;
  --font-ui:   'Rajdhani', 'Segoe UI', sans-serif;
  --glow:      0 0 20px rgba(0,200,255,0.25);
  --glow-sm:   0 0 8px rgba(0,200,255,0.2);
}

html, body {
  height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-mono);
  font-size: 13px;
  overflow: hidden;
}

/* ── Dot-grid background ───────────────────────────────────────────────── */
body::before {
  content: '';
  position: fixed;
  inset: 0;
  background-image: radial-gradient(circle, #1E2438 1px, transparent 1px);
  background-size: 28px 28px;
  opacity: 0.35;
  pointer-events: none;
  z-index: 0;
}

/* ── Layout ───────────────────────────────────────────────────────────── */
#app { display: flex; height: 100vh; overflow: hidden; position: relative; z-index: 1; }

/* ── Sidebar ──────────────────────────────────────────────────────────── */
#sidebar {
  width: 260px; min-width: 260px;
  background: var(--bg3);
  border-right: 1px solid var(--border);
  display: flex; flex-direction: column;
  overflow-y: auto; overflow-x: hidden;
}

.sb-brand {
  padding: 20px 18px 16px;
  border-bottom: 1px solid var(--border);
  background: linear-gradient(135deg, #0A0C10 0%, #0F1420 100%);
}
.sb-brand-icon {
  width: 36px; height: 36px;
  background: var(--accent-dim);
  border: 1px solid var(--accent);
  border-radius: 6px;
  display: flex; align-items: center; justify-content: center;
  font-size: 1rem;
  margin-bottom: 10px;
  box-shadow: var(--glow-sm);
}
.sb-brand .logo {
  font-family: var(--font-ui);
  font-size: 1rem; font-weight: 700;
  color: var(--white);
  letter-spacing: 2px;
  text-transform: uppercase;
}
.sb-brand .sub {
  font-size: 0.6rem; color: var(--accent);
  letter-spacing: 3px; margin-top: 3px;
  font-family: var(--font-mono);
}
.sb-brand .version {
  display: inline-block;
  font-size: 0.55rem; color: var(--text2);
  letter-spacing: 1px; margin-top: 6px;
  padding: 2px 6px;
  border: 1px solid var(--border2);
  border-radius: 2px;
}

/* ── Status dot ───────────────────────────────────────────────────────── */
.status-line {
  display: flex; align-items: center; gap: 8px;
  padding: 10px 18px;
  background: var(--green-dim);
  border-bottom: 1px solid var(--border);
  font-size: 0.65rem; color: var(--green);
  letter-spacing: 2px;
}
.pulse-dot {
  width: 7px; height: 7px;
  background: var(--green);
  border-radius: 50%;
  box-shadow: 0 0 6px var(--green);
  animation: pulse 1.8s ease-in-out infinite;
}
@keyframes pulse {
  0%, 100% { opacity: 1; box-shadow: 0 0 6px var(--green); }
  50% { opacity: 0.4; box-shadow: 0 0 2px var(--green); }
}

/* ── Nav section label ────────────────────────────────────────────────── */
.sb-section {
  padding: 14px 18px 6px;
  font-size: 0.58rem;
  color: var(--text2);
  letter-spacing: 3px;
  text-transform: uppercase;
  font-family: var(--font-mono);
}

/* ── Nav buttons ──────────────────────────────────────────────────────── */
.nav-btn {
  display: flex; align-items: center; gap: 10px;
  width: 100%; text-align: left;
  padding: 10px 18px;
  border: none; background: transparent;
  color: var(--text2);
  font-family: var(--font-mono);
  font-size: 0.75rem;
  letter-spacing: 1px;
  cursor: pointer;
  border-left: 2px solid transparent;
  transition: all 0.2s;
  position: relative;
}
.nav-btn .nav-icon { font-size: 0.85rem; min-width: 16px; }
.nav-btn:hover { background: rgba(0,200,255,0.06); color: var(--white); }
.nav-btn.active {
  background: rgba(0,200,255,0.1);
  color: var(--accent);
  border-left-color: var(--accent);
  font-weight: 600;
}
.nav-btn.active::after {
  content: '';
  position: absolute;
  right: 0; top: 0; bottom: 0;
  width: 1px;
  background: var(--accent);
  box-shadow: -2px 0 8px var(--accent);
}

/* ── Sidebar time filter ──────────────────────────────────────────────── */
.sb-divider {
  height: 1px;
  background: var(--border);
  margin: 8px 0;
}
.sb-time { padding: 12px 18px; }
.sb-time label {
  display: block;
  font-size: 0.58rem; color: var(--accent);
  letter-spacing: 2px; margin-bottom: 5px;
  text-transform: uppercase;
  font-family: var(--font-mono);
}
.sb-time input, .sb-time select {
  width: 100%; padding: 7px 10px;
  background: var(--bg);
  border: 1px solid var(--border2);
  border-radius: 3px;
  color: var(--text);
  font-family: var(--font-mono);
  font-size: 0.72rem;
  margin-bottom: 8px;
  outline: none;
  transition: border-color 0.2s;
  -webkit-appearance: none;
}
.sb-time input:focus, .sb-time select:focus {
  border-color: var(--accent);
  box-shadow: 0 0 0 2px rgba(0,200,255,0.1);
}
.sb-time select option { background: var(--bg3); }

.sb-epoch-display {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 3px;
  padding: 10px 12px;
  font-size: 0.65rem;
  color: var(--text2);
  font-family: var(--font-mono);
  line-height: 1.8;
  margin-top: 4px;
}
.sb-epoch-display .ep-key { color: var(--accent); }

/* ── Sidebar footer ───────────────────────────────────────────────────── */
.sb-footer {
  margin-top: auto;
  padding: 12px 18px;
  border-top: 1px solid var(--border);
  font-size: 0.6rem;
  color: var(--text2);
  letter-spacing: 1px;
  line-height: 1.8;
}
.sb-footer .clock { color: var(--accent); font-size: 0.65rem; }

/* ── Main content ─────────────────────────────────────────────────────── */
#main { flex: 1; overflow-y: auto; display: flex; flex-direction: column; }

/* ── Header bar ───────────────────────────────────────────────────────── */
.halo-header {
  background: linear-gradient(90deg, #070A10 0%, #0D1520 50%, #0A1018 100%);
  border-bottom: 1px solid var(--border);
  padding: 0 24px;
  height: 58px;
  display: flex; align-items: center; gap: 14px;
  position: sticky; top: 0; z-index: 10;
  box-shadow: 0 2px 20px rgba(0,0,0,0.5);
}
.halo-header::after {
  content: '';
  position: absolute;
  bottom: 0; left: 0; right: 0;
  height: 1px;
  background: linear-gradient(90deg, transparent, var(--accent), transparent);
  opacity: 0.5;
}
.header-icon { font-size: 1.1rem; }
.halo-header h1 {
  color: var(--white);
  font-family: var(--font-ui);
  font-size: 1.1rem;
  letter-spacing: 3px;
  text-transform: uppercase;
  font-weight: 600;
  flex: 1;
}
.halo-badge {
  background: transparent;
  color: var(--accent);
  font-size: 0.6rem;
  padding: 3px 10px;
  border: 1px solid var(--accent);
  border-radius: 2px;
  font-weight: 600;
  letter-spacing: 2px;
  box-shadow: var(--glow-sm);
}
.halo-badge.green { color: var(--green); border-color: var(--green); box-shadow: 0 0 8px rgba(0,232,122,0.2); }
.halo-badge.orange { color: var(--orange); border-color: var(--orange); }

/* ── Content wrapper ──────────────────────────────────────────────────── */
.content { padding: 24px; flex: 1; }

/* ── Section title ────────────────────────────────────────────────────── */
.section-title {
  font-family: var(--font-mono);
  color: var(--accent);
  font-size: 0.65rem;
  letter-spacing: 4px;
  text-transform: uppercase;
  margin-bottom: 14px;
  margin-top: 8px;
  display: flex; align-items: center; gap: 10px;
}
.section-title::after {
  content: '';
  flex: 1;
  height: 1px;
  background: linear-gradient(90deg, var(--border), transparent);
}

/* ── Metric cards ─────────────────────────────────────────────────────── */
.metric-row { display: flex; gap: 14px; margin-bottom: 24px; flex-wrap: wrap; }
.metric-card {
  flex: 1; min-width: 140px;
  background: var(--bg2);
  border: 1px solid var(--border);
  border-top: 2px solid var(--accent);
  border-radius: 4px;
  padding: 16px 18px;
  position: relative;
  overflow: hidden;
  transition: transform 0.2s, box-shadow 0.2s;
}
.metric-card::before {
  content: '';
  position: absolute;
  top: 0; right: 0;
  width: 60px; height: 60px;
  background: radial-gradient(circle at top right, var(--accent-dim), transparent);
  pointer-events: none;
}
.metric-card:hover { transform: translateY(-2px); box-shadow: 0 4px 20px rgba(0,0,0,0.4); }
.metric-card.green  { border-top-color: var(--green); }
.metric-card.green::before { background: radial-gradient(circle at top right, var(--green-dim), transparent); }
.metric-card.orange { border-top-color: var(--orange); }
.metric-card.orange::before { background: radial-gradient(circle at top right, var(--orange-dim), transparent); }
.metric-card.red    { border-top-color: var(--red); }
.metric-card.red::before { background: radial-gradient(circle at top right, var(--red-dim), transparent); }
.metric-card .mc-label {
  font-size: 0.58rem; color: var(--text2);
  letter-spacing: 3px; text-transform: uppercase;
  font-family: var(--font-mono);
}
.metric-card .mc-value {
  font-family: var(--font-ui);
  font-size: 1.6rem;
  color: var(--white);
  font-weight: 700;
  margin-top: 6px;
  letter-spacing: 1px;
}
.metric-card .mc-sub {
  font-size: 0.6rem; color: var(--text2);
  margin-top: 3px;
  letter-spacing: 1px;
}

/* ── Buttons ──────────────────────────────────────────────────────────── */
.btn {
  background: transparent;
  color: var(--accent);
  border: 1px solid var(--accent);
  border-radius: 3px;
  font-weight: 600;
  letter-spacing: 2px;
  font-size: 0.7rem;
  padding: 10px 24px;
  cursor: pointer;
  font-family: var(--font-mono);
  text-transform: uppercase;
  transition: all 0.2s;
  position: relative;
  overflow: hidden;
}
.btn::before {
  content: '';
  position: absolute;
  inset: 0;
  background: var(--accent);
  opacity: 0;
  transition: opacity 0.2s;
}
.btn:hover {
  color: var(--bg);
  box-shadow: var(--glow);
}
.btn:hover::before { opacity: 1; }
.btn > span { position: relative; z-index: 1; }

/* ── Input group ──────────────────────────────────────────────────────── */
.input-row { display: flex; gap: 12px; align-items: flex-end; margin-bottom: 20px; max-width: 620px; }
.input-group { flex: 1; }
.input-group label {
  display: block;
  font-size: 0.58rem; color: var(--text2);
  letter-spacing: 3px; margin-bottom: 6px;
  text-transform: uppercase; font-family: var(--font-mono);
}
.input-group input {
  width: 100%; padding: 10px 14px;
  background: var(--bg3);
  border: 1px solid var(--border2);
  border-radius: 3px;
  color: var(--white);
  font-family: var(--font-mono);
  font-size: 0.85rem;
  outline: none;
  transition: all 0.2s;
}
.input-group input:focus {
  border-color: var(--accent);
  background: var(--bg);
  box-shadow: 0 0 0 3px rgba(0,200,255,0.08);
}

/* ── Tabs ─────────────────────────────────────────────────────────────── */
.tab-bar {
  display: flex; gap: 0;
  border-bottom: 1px solid var(--border);
  margin-bottom: 20px;
}
.tab-btn {
  padding: 10px 22px;
  border: none; background: transparent;
  color: var(--text2);
  font-family: var(--font-mono);
  font-size: 0.72rem;
  letter-spacing: 1.5px;
  text-transform: uppercase;
  cursor: pointer;
  border-bottom: 2px solid transparent;
  margin-bottom: -1px;
  transition: all 0.2s;
}
.tab-btn:hover { color: var(--text); }
.tab-btn.active {
  color: var(--accent);
  border-bottom-color: var(--accent);
  font-weight: 600;
}
.tab-panel { display: none; }
.tab-panel.active { display: block; animation: fadeIn 0.25s ease; }
@keyframes fadeIn { from { opacity: 0; transform: translateY(4px); } to { opacity: 1; transform: translateY(0); } }

/* ── Data table ───────────────────────────────────────────────────────── */
.data-table-wrap {
  border: 1px solid var(--border);
  border-radius: 4px;
  max-height: 480px;
  overflow-y: auto;
  background: var(--bg3);
}
.data-table { width: 100%; border-collapse: collapse; font-size: 0.75rem; }
.data-table thead th {
  position: sticky; top: 0; z-index: 2;
  background: var(--bg);
  color: var(--accent);
  text-align: left;
  padding: 10px 14px;
  font-size: 0.58rem;
  letter-spacing: 3px;
  text-transform: uppercase;
  border-bottom: 1px solid var(--border);
  font-family: var(--font-mono);
  white-space: nowrap;
}
.data-table tbody tr { transition: background 0.15s; }
.data-table tbody tr:nth-child(even) { background: rgba(0,0,0,0.15); }
.data-table tbody tr:hover { background: rgba(0,200,255,0.06); }
.data-table td {
  padding: 9px 14px;
  border-bottom: 1px solid rgba(30,36,56,0.5);
  color: var(--text);
  font-family: var(--font-mono);
}
.data-table td:first-child { color: var(--text2); }

/* ── Bar chart ────────────────────────────────────────────────────────── */
.bar-chart { display: flex; flex-direction: column; gap: 10px; }
.bar-row { display: flex; align-items: center; gap: 12px; font-size: 0.72rem; }
.bar-rank { width: 18px; text-align: right; color: var(--text2); font-size: 0.65rem; flex-shrink: 0; }
.bar-label { width: 80px; text-align: right; color: var(--text); font-size: 0.72rem; flex-shrink: 0; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
.bar-track { flex: 1; background: rgba(0,0,0,0.3); border-radius: 2px; height: 22px; overflow: hidden; border: 1px solid var(--border); }
.bar-fill {
  height: 100%;
  background: linear-gradient(90deg, var(--accent2), var(--accent));
  border-radius: 2px;
  transition: width 0.6s cubic-bezier(.4,0,.2,1);
  min-width: 0;
}
.bar-count {
  min-width: 44px;
  text-align: right;
  color: var(--white);
  font-size: 0.72rem;
  font-family: var(--font-mono);
  flex-shrink: 0;
}

/* ── SVG area chart ───────────────────────────────────────────────────── */
.chart-wrap {
  background: var(--bg2);
  border: 1px solid var(--border);
  border-radius: 4px;
  padding: 16px;
}
.chart-wrap svg { width: 100%; display: block; }

/* ── Progress bar in table ────────────────────────────────────────────── */
.progress-cell { display: flex; align-items: center; gap: 10px; }
.progress-track { flex: 1; height: 8px; background: var(--bg); border-radius: 2px; overflow: hidden; }
.progress-fill {
  height: 100%;
  background: linear-gradient(90deg, var(--accent2), var(--accent));
  border-radius: 2px;
  transition: width 0.5s ease;
}

/* ── Alert boxes ──────────────────────────────────────────────────────── */
.alert-box {
  background: rgba(255,140,0,0.06);
  border: 1px solid rgba(255,140,0,0.3);
  border-left: 3px solid var(--orange);
  border-radius: 3px;
  padding: 14px 18px;
  margin: 8px 0;
  font-size: 0.78rem;
  display: flex; flex-direction: column; gap: 0;
}
.alert-box.critical {
  background: rgba(255,60,90,0.06);
  border-color: rgba(255,60,90,0.3);
  border-left-color: var(--red);
}
.alert-box .al-header {
  display: flex; align-items: center; gap: 10px;
  margin-bottom: 6px;
}
.al-sev {
  font-size: 0.58rem;
  padding: 2px 7px;
  border-radius: 2px;
  font-weight: bold;
  letter-spacing: 2px;
  font-family: var(--font-mono);
}
.al-sev.warning { background: var(--orange-dim); color: var(--orange); border: 1px solid rgba(255,140,0,0.3); }
.al-sev.critical { background: var(--red-dim); color: var(--red); border: 1px solid rgba(255,60,90,0.3); }
.al-tag {
  margin-left: auto;
  font-size: 0.55rem;
  padding: 2px 8px;
  border-radius: 2px;
  font-weight: bold;
  letter-spacing: 2px;
  font-family: var(--font-mono);
}
.al-tag.planned { background: var(--accent-dim); color: var(--accent); border: 1px solid rgba(0,200,255,0.3); }
.al-tag.advanced { background: var(--orange-dim); color: var(--orange); border: 1px solid rgba(255,140,0,0.3); }
.alert-box .al-title {
  font-weight: 600;
  color: var(--white);
  font-family: var(--font-ui);
  font-size: 0.85rem;
  letter-spacing: 1px;
}
.alert-box .al-body { color: var(--text2); font-size: 0.73rem; line-height: 1.6; }

/* ── Spinner ──────────────────────────────────────────────────────────── */
.spinner-wrap { padding: 60px; text-align: center; color: var(--accent); }
.spinner {
  display: inline-block; width: 32px; height: 32px;
  border: 2px solid var(--border);
  border-top-color: var(--accent);
  border-right-color: var(--accent);
  border-radius: 50%;
  animation: spin 0.7s linear infinite;
  box-shadow: var(--glow-sm);
}
.spinner-label { margin-top: 14px; font-size: 0.72rem; letter-spacing: 2px; color: var(--text2); }
@keyframes spin { to { transform: rotate(360deg); } }

/* ── Messages ─────────────────────────────────────────────────────────── */
.msg { padding: 14px 18px; border-radius: 3px; font-size: 0.78rem; margin-bottom: 16px; line-height: 1.6; }
.msg.error   { background: var(--red-dim);    border: 1px solid rgba(255,60,90,0.3);  color: #ff9aaa; }
.msg.warning { background: var(--orange-dim); border: 1px solid rgba(255,140,0,0.3); color: #ffcc80; }
.msg.success { background: var(--green-dim);  border: 1px solid rgba(0,232,122,0.3); color: #80ffc0; }
.msg.info    { background: var(--accent-dim); border: 1px solid rgba(0,200,255,0.3); color: #80e0ff; }
.msg code    { background: rgba(0,0,0,0.4); padding: 4px 10px; border-radius: 3px; font-family: var(--font-mono); display: block; margin-top: 8px; font-size: 0.75rem; color: var(--text2); }

/* ── Event type badge ─────────────────────────────────────────────────── */
.ev-badge {
  display: inline-block;
  font-size: 0.62rem;
  padding: 2px 7px;
  border-radius: 2px;
  letter-spacing: 1px;
  font-family: var(--font-mono);
  background: var(--bg);
  border: 1px solid var(--border2);
  color: var(--text);
}
.ev-badge.login     { color: var(--green);  border-color: rgba(0,232,122,0.3); background: var(--green-dim); }
.ev-badge.logout    { color: var(--text2);  }
.ev-badge.failed    { color: var(--red);    border-color: rgba(255,60,90,0.3); background: var(--red-dim); }
.ev-badge.admin     { color: var(--orange); border-color: rgba(255,140,0,0.3); background: var(--orange-dim); }
.ev-badge.download  { color: var(--accent); border-color: rgba(0,200,255,0.3); background: var(--accent-dim); }

/* ── Location flag ────────────────────────────────────────────────────── */
.loc-badge { font-size: 0.72rem; color: var(--text2); }

/* ── Module visibility ────────────────────────────────────────────────── */
.module { display: none; }
.module.active { display: flex; flex-direction: column; flex: 1; animation: fadeIn 0.3s ease; }

/* ── Scrollbar ────────────────────────────────────────────────────────── */
::-webkit-scrollbar { width: 6px; height: 6px; }
::-webkit-scrollbar-track { background: var(--bg3); }
::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 3px; }
::-webkit-scrollbar-thumb:hover { background: var(--accent2); }

hr { border: none; border-top: 1px solid var(--border); margin: 20px 0; }

/* ── Grid layout for breakdown panels ────────────────────────────────── */
.two-col { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }

/* ── Scan-line overlay (subtle) ───────────────────────────────────────── */
#main::before {
  content: '';
  position: fixed;
  top: 0; left: 260px; right: 0; bottom: 0;
  background: repeating-linear-gradient(
    0deg,
    transparent,
    transparent 2px,
    rgba(0,0,0,0.015) 2px,
    rgba(0,0,0,0.015) 4px
  );
  pointer-events: none;
  z-index: 100;
}

/* ── Anomaly radar page ───────────────────────────────────────────────── */
.radar-grid { display: grid; gap: 10px; }

/* ── Quality page cat cards ───────────────────────────────────────────── */
.cat-row { display: grid; grid-template-columns: repeat(3, 1fr); gap: 14px; margin-bottom: 20px; }
</style>
</head>
<body>
<div id="app">

  <!-- ═══════════════════════ SIDEBAR ═══════════════════════ -->
  <div id="sidebar">
    <div class="sb-brand">
      <div class="sb-brand-icon">⬡</div>
      <div class="logo">HALO SOC</div>
      <div class="sub">CYBER ACCESS ENGINE</div>
      <span class="version">v2.1.0 &nbsp;·&nbsp; IN-MEMORY MODE</span>
    </div>

    <div class="status-line">
      <div class="pulse-dot"></div>
      ENGINE ONLINE &nbsp;·&nbsp; PORT 8080
    </div>

    <button class="nav-btn" onclick="showModule('quality', this)">
      <span class="nav-icon">◰</span> Data Quality
    </button>
    <button class="nav-btn active" onclick="showModule('overview', this)">
      <span class="nav-icon">◈</span> Overview: Top 10
    </button>
    <button class="nav-btn" onclick="showModule('user', this)">
      <span class="nav-icon">◎</span> User Journey
    </button>
    <button class="nav-btn" onclick="showModule('resource', this)">
      <span class="nav-icon">◉</span> Resource Tracker
    </button>
    <button class="nav-btn" onclick="showModule('anomaly', this)">
      <span class="nav-icon">◬</span> Anomaly Radar
    </button>

    <div class="sb-divider"></div>
    <div class="sb-time">
      <label>Input Format</label>
      <select id="timeMode" onchange="updateTimeMode()">
        <option value="date">Standard Date</option>
        <option value="epoch">Epoch Timestamp</option>
      </select>

      <div id="dateInputs">
        <label>From</label>
        <input type="date" id="startDate" value="2024-04-13" onchange="updateEpochDisplay()">
        <label>To</label>
        <input type="date" id="endDate" value="2025-04-15" onchange="updateEpochDisplay()">
      </div>

      <div id="epochInputs" style="display:none">
        <label>From (Epoch)</label>
        <input type="number" id="startEpoch" value="1712966400" onchange="updateEpochDisplay()">
        <label>To (Epoch)</label>
        <input type="number" id="endEpoch" value="1744675200" onchange="updateEpochDisplay()">
      </div>

      <div class="sb-epoch-display">
        <span class="ep-key">START_TS</span> › <span id="displayStart">1712966400</span><br>
        <span class="ep-key">END_TS&nbsp;&nbsp;</span> › <span id="displayEnd">1744675200</span>
      </div>
    </div>

    <div class="sb-footer">
      <div class="clock" id="liveClock">──:──:── UTC</div>
      <div style="margin-top:4px">API · LOCALHOST:8080</div>
    </div>
  </div>

  <!-- ═══════════════════════ MAIN ═══════════════════════ -->
  <div id="main">

    <!-- MODULE 1 — OVERVIEW -->
    <div id="mod-overview" class="module active">
      <div class="halo-header">
        <span class="header-icon">◈</span>
        <h1>Overview Dashboard</h1>
        <span class="halo-badge green">LIVE</span>
      </div>
      <div class="content">
        <div class="metric-row">
          <div class="metric-card">
            <div class="mc-label">Dataset</div>
            <div class="mc-value" id="ov-dataset">—</div>
            <div class="mc-sub">log entries loaded</div>
          </div>
          <div class="metric-card green">
            <div class="mc-label">Engine</div>
            <div class="mc-value">ONLINE</div>
            <div class="mc-sub">C++ In-Memory v2</div>
          </div>
          <div class="metric-card orange">
            <div class="mc-label">RAM Usage</div>
            <div class="mc-value">~120 MB</div>
            <div class="mc-sub">memory pool active</div>
          </div>
          <div class="metric-card">
            <div class="mc-label">API P50</div>
            <div class="mc-value">&lt; 2 ms</div>
            <div class="mc-sub">query latency</div>
          </div>
        </div>

        <div class="section-title">Top 10 Most-Accessed Resources</div>
        <div style="margin-bottom:20px">
          <button class="btn" onclick="loadTop10()"><span>⚡ EXECUTE QUERY</span></button>
        </div>
        <div id="top10-result"></div>
      </div>
    </div>

    <!-- MODULE 2 — USER JOURNEY -->
    <div id="mod-user" class="module">
      <div class="halo-header">
        <span class="header-icon">◎</span>
        <h1>User Journey Investigation</h1>
        <span class="halo-badge">LIVE</span>
      </div>
      <div class="content">
        <div class="section-title">Target User Search</div>
        <div class="input-row">
          <div class="input-group">
            <label>User ID</label>
            <input type="text" id="userId" value="U03649" placeholder="e.g. U03649">
          </div>
          <button class="btn" onclick="loadUser()"><span>⟶ TRACK USER</span></button>
        </div>
        <div id="user-result"></div>
      </div>
    </div>

    <!-- MODULE 3 — RESOURCE TRACKER -->
    <div id="mod-resource" class="module">
      <div class="halo-header">
        <span class="header-icon">◉</span>
        <h1>Resource Access Tracker</h1>
        <span class="halo-badge">LIVE</span>
      </div>
      <div class="content">
        <div class="section-title">Target Resource Search</div>
        <div class="input-row">
          <div class="input-group">
            <label>Resource ID</label>
            <input type="text" id="resourceId" value="R03922" placeholder="e.g. R03922">
          </div>
          <button class="btn" onclick="loadResource()"><span>⟶ AUDIT RESOURCE</span></button>
        </div>
        <div id="resource-result"></div>
      </div>
    </div>

    <!-- MODULE 4 — ANOMALY RADAR -->
    <div id="mod-anomaly" class="module">
      <div class="halo-header">
        <span class="header-icon">◬</span>
        <h1>Anomaly Detection Radar</h1>
        <span class="halo-badge orange">MODULE 4</span>
      </div>
      <div class="content">
        <div class="section-title">Detection Rules Deployment Status</div>
        <div id="anomaly-modules" class="radar-grid"></div>
        <hr>
        <div class="msg info">
          ⓘ &nbsp;Telemetry for these modules requires integration of secondary C++ Hash Tables (Device Indexing) — scheduled for Phase 4 deployment.
        </div>
      </div>
    </div>

    <!-- MODULE 5 — DATA QUALITY -->
    <div id="mod-quality" class="module">
      <div class="halo-header">
        <span class="header-icon">◰</span>
        <h1>Data Quality Report</h1>
        <span class="halo-badge">INGEST</span>
      </div>
      <div class="content">
        <div id="quality-result">
          <div class="spinner-wrap">
            <div class="spinner"></div>
            <div class="spinner-label">FETCHING QUALITY REPORT FROM ENGINE...</div>
          </div>
        </div>
      </div>
    </div>

  </div><!-- /#main -->
</div><!-- /#app -->

<script>
// ═══════════════════════════ UTILITY ════════════════════════════════════════

function getEpochs() {
  const mode = document.getElementById('timeMode').value;
  if (mode === 'date') {
    const s = new Date(document.getElementById('startDate').value + 'T00:00:00Z');
    const e = new Date(document.getElementById('endDate').value   + 'T23:59:59Z');
    return [Math.floor(s.getTime()/1000), Math.floor(e.getTime()/1000)];
  }
  return [
    parseInt(document.getElementById('startEpoch').value) || 0,
    parseInt(document.getElementById('endEpoch').value)   || 9999999999
  ];
}

function updateEpochDisplay() {
  const [s, e] = getEpochs();
  document.getElementById('displayStart').textContent = s;
  document.getElementById('displayEnd').textContent   = e;
}

function updateTimeMode() {
  const mode = document.getElementById('timeMode').value;
  document.getElementById('dateInputs').style.display  = mode === 'date'  ? 'block' : 'none';
  document.getElementById('epochInputs').style.display = mode === 'epoch' ? 'block' : 'none';
  updateEpochDisplay();
}

function fmtEpoch(ts) {
  return new Date(ts * 1000).toISOString().replace('T',' ').slice(0,19) + ' UTC';
}

function loading(msg = 'Processing...') {
  return `<div class="spinner-wrap"><div class="spinner"></div><div class="spinner-label">${msg.toUpperCase()}</div></div>`;
}

function errBox(msg) {
  return `<div class="msg error">⚠ &nbsp;${msg}<code>$ ./release/halo_engine  # Ensure backend is running on port 8080</code></div>`;
}

async function apiFetch(path) {
  try {
    const r = await fetch(path, { signal: AbortSignal.timeout(15000) });
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return [await r.json(), null];
  } catch(e) {
    if (e.name === 'TimeoutError') return [null, 'TIMEOUT: Request to engine timed out after 15s.'];
    if (e.name === 'TypeError')    return [null, 'CRITICAL: Cannot connect to C++ Engine on port 8080.'];
    return [null, 'SYSTEM ERROR: ' + e.message];
  }
}

async function initDatasetCount() {
  const [data] = await apiFetch('/api/quality');
  if (data && data.loaded) {
    document.getElementById('ov-dataset').textContent = data.loaded.toLocaleString();
  }
}

// ═══════════════════════ LIVE CLOCK ════════════════════════════════════════
function tickClock() {
  const now = new Date();
  const hh = String(now.getUTCHours()).padStart(2,'0');
  const mm = String(now.getUTCMinutes()).padStart(2,'0');
  const ss = String(now.getUTCSeconds()).padStart(2,'0');
  document.getElementById('liveClock').textContent = `${hh}:${mm}:${ss} UTC`;
}
setInterval(tickClock, 1000); tickClock();

// ═══════════════════════ NAVIGATION ════════════════════════════════════════
function showModule(name, btn) {
  document.querySelectorAll('.module').forEach(m => m.classList.remove('active'));
  document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));
  document.getElementById('mod-' + name).classList.add('active');
  if (btn) btn.classList.add('active');
  if (name === 'quality') loadQuality();
  if (name === 'anomaly') renderAnomalyModules();
}

// ═══════════════════════ HELPERS ════════════════════════════════════════════
function metricCard(label, value, sub = '', color = '') {
  return `<div class="metric-card ${color}">
    <div class="mc-label">${label}</div>
    <div class="mc-value">${value}</div>
    <div class="mc-sub">${sub}</div>
  </div>`;
}

function makeTabs(id, tabs) {
  const btnHtml = tabs.map((t,i) =>
    `<button class="tab-btn ${i===0?'active':''}" onclick="switchTab('${id}',${i},this)">${t.label}</button>`
  ).join('');
  const panelHtml = tabs.map((t,i) =>
    `<div class="tab-panel ${i===0?'active':''}" id="${id}-panel-${i}">${t.content}</div>`
  ).join('');
  return `<div class="tab-bar">${btnHtml}</div>${panelHtml}`;
}

function switchTab(id, idx, btn) {
  document.querySelectorAll(`[id^="${id}-panel-"]`).forEach(p => p.classList.remove('active'));
  btn.closest('.tab-bar').querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  document.getElementById(`${id}-panel-${idx}`).classList.add('active');
  btn.classList.add('active');
}

// Event type labels + badge classes
const EVENT_LABELS = ['LOGIN','LOGOUT','TOKEN_REFRESH','ACCESS','FAILED_LOGIN','OPEN_APP','DOWNLOAD','ADMIN_ACTION','UNKNOWN'];
const EVENT_CLASS  = ['login','logout','','','failed','','download','admin',''];
const LOC_FLAGS    = {US:'🇺🇸',VN:'🇻🇳',JP:'🇯🇵',KR:'🇰🇷',SG:'🇸🇬',CN:'🇨🇳',DE:'🇩🇪',FR:'🇫🇷',UK:'🇬🇧',AU:'🇦🇺',CA:'🇨🇦',IN:'🇮🇳',BR:'🇧🇷',RU:'🇷🇺',TH:'🇹🇭'};
const LOC_NAMES    = ['US','VN','JP','KR','SG','CN','DE','FR','UK','AU','CA','IN','BR','RU','TH','UNKNOWN'];

function evBadge(ev) {
  const name = EVENT_LABELS[ev] || String(ev);
  const cls  = EVENT_CLASS[ev]  || '';
  return `<span class="ev-badge ${cls}">${name}</span>`;
}
function locBadge(loc) {
  const name = LOC_NAMES[loc] || String(loc);
  const flag = LOC_FLAGS[name] || '';
  return `<span class="loc-badge">${flag} ${name}</span>`;
}

// ═══════════════════════ CHARTS ════════════════════════════════════════════
function toHourlyBuckets(rows, tsField) {
  const buckets = {};
  rows.forEach(r => {
    const h = Math.floor(r[tsField] / 3600) * 3600;
    buckets[h] = (buckets[h] || 0) + 1;
  });
  return Object.keys(buckets).sort((a,b)=>+a- +b).map(t=>({t:+t, v:buckets[t]}));
}

function makeAreaChart(data, color = '#00C8FF', height = 200) {
  if (!data || data.length === 0) return '<div class="msg info" style="margin:0">No timeline data available.</div>';
  const W = 900, H = height, P = {top:16, right:16, bottom:28, left:46};
  const pw = W - P.left - P.right, ph = H - P.top - P.bottom;
  const vals  = data.map(d => d.v);
  const times = data.map(d => d.t);
  const maxV  = Math.max(...vals) || 1;
  const minT  = times[0], maxT = times[times.length-1] || minT+1;
  const sx = t => P.left + (t - minT) / (maxT - minT || 1) * pw;
  const sy = v => P.top  + (1 - v/maxV) * ph;
  const pts = data.map(d => `${sx(d.t)},${sy(d.v)}`).join(' ');
  const area = `M${sx(minT)},${P.top+ph} ` + data.map(d=>`L${sx(d.t)},${sy(d.v)}`).join(' ') + ` L${sx(maxT)},${P.top+ph} Z`;
  const gid  = 'g'+color.replace('#','');
  const xTk  = Array.from({length:5},(_,i)=>{
    const t = minT + (maxT-minT)*i/4;
    const x = sx(t);
    const lb = new Date(t*1000).toLocaleDateString('en',{month:'short',day:'numeric'});
    return `<text x="${x}" y="${H-4}" text-anchor="middle" fill="#8B90A8" font-size="9" font-family="Share Tech Mono,monospace">${lb}</text>
            <line x1="${x}" y1="${P.top}" x2="${x}" y2="${P.top+ph}" stroke="#1E2438" stroke-width="1"/>`;
  }).join('');
  const yTk  = Array.from({length:4},(_,i)=>{
    const v = Math.round(maxV*i/3);
    const y = sy(v);
    return `<text x="${P.left-6}" y="${y+4}" text-anchor="end" fill="#8B90A8" font-size="9" font-family="Share Tech Mono,monospace">${v}</text>
            <line x1="${P.left}" y1="${y}" x2="${P.left+pw}" y2="${y}" stroke="#1E2438" stroke-width="1"/>`;
  }).join('');
  return `<div class="chart-wrap"><svg viewBox="0 0 ${W} ${H}" preserveAspectRatio="none">
    <defs>
      <linearGradient id="${gid}" x1="0" y1="0" x2="0" y2="1">
        <stop offset="0%" stop-color="${color}" stop-opacity="0.4"/>
        <stop offset="100%" stop-color="${color}" stop-opacity="0.02"/>
      </linearGradient>
    </defs>
    ${xTk}${yTk}
    <path d="${area}" fill="url(#${gid})"/>
    <polyline points="${pts}" fill="none" stroke="${color}" stroke-width="1.5"/>
    ${data.map(d=>`<circle cx="${sx(d.t)}" cy="${sy(d.v)}" r="2" fill="${color}" opacity="0.6"/>`).join('')}
  </svg></div>`;
}

function makeBarChart(entries, color = 'var(--accent)') {
  if (!entries.length) return '<div class="msg info">No data.</div>';
  const max = entries[0][1] || 1;
  return `<div class="bar-chart">` + entries.map(([k,v],i)=>`
    <div class="bar-row">
      <span class="bar-rank">${i+1}.</span>
      <span class="bar-label" title="${k}">${k}</span>
      <div class="bar-track">
        <div class="bar-fill" style="width:${Math.max(v/max*100,1)}%;background:linear-gradient(90deg,${color}99,${color})"></div>
      </div>
      <span class="bar-count">${v.toLocaleString()}</span>
    </div>`).join('') + `</div>`;
}

// ═══════════════════════ MODULE 1 — TOP 10 ═════════════════════════════════
async function loadTop10() {
  const el = document.getElementById('top10-result');
  el.innerHTML = loading('Executing query on C++ engine...');
  const [s, e] = getEpochs();
  const [data, err] = await apiFetch(`/api/top?start=${s}&end=${e}`);
  if (err) { el.innerHTML = errBox(err); return; }
  if (!data || data.length === 0) { el.innerHTML = '<div class="msg warning">No resources found in the specified time range.</div>'; return; }

  const maxHits = Math.max(...data.map(d=>d.count));
  const total   = data.reduce((a,d)=>a+d.count,0);

  const tableRows = data.map((d,i) => `<tr>
    <td style="color:var(--text2);width:30px">${i+1}</td>
    <td style="color:var(--accent);font-weight:600">${d.resource_id}</td>
    <td>
      <div class="progress-cell">
        <div class="progress-track"><div class="progress-fill" style="width:${d.count/maxHits*100}%"></div></div>
        <span style="min-width:50px;color:var(--white)">${d.count.toLocaleString()}</span>
      </div>
    </td>
    <td style="color:var(--text2)">${(d.count/total*100).toFixed(1)}%</td>
  </tr>`).join('');

  const barEntries = data.map(d=>[d.resource_id, d.count]);
  const bars = makeBarChart(barEntries);

  const tabs = makeTabs('top10', [
    { label: '◫ Access Frequency Table', content: `
        <div class="data-table-wrap">
          <table class="data-table">
            <thead><tr><th>#</th><th>Resource ID</th><th>Access Count</th><th>Share %</th></tr></thead>
            <tbody>${tableRows}</tbody>
          </table>
        </div>` },
    { label: '▦ Frequency Distribution', content: `<div class="chart-wrap" style="padding:24px">${bars}</div>` }
  ]);

  const metrics = `<div class="metric-row">
    ${metricCard('Total Accesses', total.toLocaleString(), `${fmtEpoch(s)} → ${fmtEpoch(e)}`)}
    ${metricCard('Top Resource', data[0].resource_id, `${data[0].count.toLocaleString()} hits`, 'green')}
    ${metricCard('Resources Tracked', data.length, 'unique IDs in top 10')}
  </div>`;

  el.innerHTML = metrics + '<hr>' + tabs;
}

// ═══════════════════════ MODULE 2 — USER JOURNEY ═══════════════════════════
async function loadUser() {
  const el  = document.getElementById('user-result');
  const uid = document.getElementById('userId').value.trim();
  if (!uid) return;
  el.innerHTML = loading(`Compiling journey for ${uid}...`);
  const [s,e] = getEpochs();
  const [data, err] = await apiFetch(`/api/user?id=${encodeURIComponent(uid)}&start=${s}&end=${e}`);
  if (err) { el.innerHTML = errBox(err); return; }
  if (!data || data.length === 0) { el.innerHTML = `<div class="msg warning">No activity logs found for user <strong>${uid}</strong> in the specified time range.</div>`; return; }

  const devices   = new Set(data.map(d=>d.device_id)).size;
  const apps      = new Set(data.map(d=>d.app_id)).size;
  const resources = new Set(data.map(d=>d.resource_id)).size;
  const tsArr     = data.map(d=>d.timestamp);
  const spanH     = ((Math.max(...tsArr) - Math.min(...tsArr)) / 3600).toFixed(1);

  const metrics = `<div class="metric-row">
    ${metricCard('Events',    data.length.toLocaleString(), 'in time range')}
    ${metricCard('Devices',   devices,   'unique devices',   devices>3?'orange':'')}
    ${metricCard('Apps',      apps,      'unique apps')}
    ${metricCard('Resources', resources, 'unique resources')}
    ${metricCard('Span',      spanH+'h', 'activity window')}
  </div>`;

  const logRows = data.map(r => `<tr>
    <td>${fmtEpoch(r.timestamp)}</td>
    <td style="color:var(--accent);font-weight:600">${r.device_id}</td>
    <td>${r.app_id}</td>
    <td>${r.resource_id}</td>
    <td>${evBadge(r.event_type)}</td>
    <td>${locBadge(r.location)}</td>
  </tr>`).join('');

  const timeline = toHourlyBuckets(data, 'timestamp');
  const chart    = makeAreaChart(timeline, '#00C8FF');

  const resCounts = {}, appCounts = {};
  data.forEach(d => { resCounts[d.resource_id]=(resCounts[d.resource_id]||0)+1; appCounts[d.app_id]=(appCounts[d.app_id]||0)+1; });
  const topRes = Object.entries(resCounts).sort((a,b)=>b[1]-a[1]).slice(0,10);
  const topApp = Object.entries(appCounts).sort((a,b)=>b[1]-a[1]).slice(0,10);

  const hints = [];
  if (devices > 4) hints.push(['HIGH DEVICE COUNT', `Account accessed from ${devices} distinct devices. Evaluate for credential sharing or compromise.`, false]);
  if (+spanH > 0 && data.length/+spanH > 500) hints.push(['ELEVATED ACTIVITY RATE', `Sustained ${(data.length/+spanH).toFixed(0)} events/hour — potential automated script behavior.`, true]);

  const anomalyHtml = hints.length === 0
    ? '<div class="msg success">✓ &nbsp;STATUS CLEAR: No standard anomalies detected for this user in the selected range.</div>'
    : hints.map(([t,b,c]) => `
      <div class="alert-box ${c?'critical':''}">
        <div class="al-header">
          <span class="al-sev ${c?'critical':'warning'}">${c?'CRITICAL':'WARNING'}</span>
          <span class="al-title">${t}</span>
        </div>
        <div class="al-body">${b}</div>
      </div>`).join('');

  const tabs = makeTabs('user', [
    { label: '◫ Event Log', content: `
        <div class="data-table-wrap">
          <table class="data-table">
            <thead><tr><th>Date &amp; Time (UTC)</th><th>Device ID</th><th>App ID</th><th>Resource ID</th><th>Event</th><th>Location</th></tr></thead>
            <tbody>${logRows}</tbody>
          </table>
        </div>` },
    { label: '▲ Activity Timeline', content: chart },
    { label: '▦ Breakdown', content: `
        <div class="two-col">
          <div>
            <div class="section-title">Top Accessed Resources</div>
            ${makeBarChart(topRes)}
          </div>
          <div>
            <div class="section-title">Top Apps Used</div>
            ${makeBarChart(topApp, 'var(--green)')}
          </div>
        </div>` }
  ]);

  el.innerHTML = metrics + tabs + '<hr><div class="section-title">System Anomaly Flags</div>' + anomalyHtml;
}

// ═══════════════════════ MODULE 3 — RESOURCE TRACKER ══════════════════════
async function loadResource() {
  const el  = document.getElementById('resource-result');
  const rid = document.getElementById('resourceId').value.trim();
  if (!rid) return;
  el.innerHTML = loading(`Compiling access logs for ${rid}...`);
  const [s,e] = getEpochs();
  const [data, err] = await apiFetch(`/api/resource?id=${encodeURIComponent(rid)}&start=${s}&end=${e}`);
  if (err) { el.innerHTML = errBox(err); return; }
  if (!data || data.length === 0) { el.innerHTML = `<div class="msg warning">No access records found for resource <strong>${rid}</strong> in the specified time range.</div>`; return; }

  const users   = new Set(data.map(d=>d.user_id)).size;
  const devices = new Set(data.map(d=>d.device_id)).size;
  const apps    = new Set(data.map(d=>d.app_id)).size;

  const metrics = `<div class="metric-row">
    ${metricCard('Total Accesses', data.length.toLocaleString(), 'in time range')}
    ${metricCard('Unique Users',   users,   'distinct accounts', users>20?'orange':'')}
    ${metricCard('Unique Devices', devices, 'distinct devices')}
    ${metricCard('Unique Apps',    apps,    'distinct apps')}
  </div>`;

  const logRows = data.map(r => `<tr>
    <td>${fmtEpoch(r.timestamp)}</td>
    <td style="color:var(--accent);font-weight:600">${r.user_id}</td>
    <td>${r.device_id}</td>
    <td>${r.app_id}</td>
    <td>${evBadge(r.event_type)}</td>
    <td>${locBadge(r.location)}</td>
  </tr>`).join('');

  const timeline = toHourlyBuckets(data, 'timestamp');
  const chart    = makeAreaChart(timeline, '#FF3C5A');

  const userCounts = {}, devCounts = {};
  data.forEach(d => { userCounts[d.user_id]=(userCounts[d.user_id]||0)+1; devCounts[d.device_id]=(devCounts[d.device_id]||0)+1; });
  const topU = Object.entries(userCounts).sort((a,b)=>b[1]-a[1]).slice(0,10);
  const topD = Object.entries(devCounts).sort((a,b)=>b[1]-a[1]).slice(0,10);

  const hints = [];
  if (users > 50) hints.push(['WIDE ACCESS DISTRIBUTION', `${users} distinct users accessed this resource. Verify resource classification allows broad access.`, false]);
  const anomalyHtml = hints.length === 0
    ? '<div class="msg success">✓ &nbsp;STATUS CLEAR: No standard anomalies detected for this resource in the selected range.</div>'
    : hints.map(([t,b,c]) => `
      <div class="alert-box ${c?'critical':''}">
        <div class="al-header">
          <span class="al-sev ${c?'critical':'warning'}">${c?'CRITICAL':'WARNING'}</span>
          <span class="al-title">${t}</span>
        </div>
        <div class="al-body">${b}</div>
      </div>`).join('');

  const tabs = makeTabs('res', [
    { label: '◫ Access Log', content: `
        <div class="data-table-wrap">
          <table class="data-table">
            <thead><tr><th>Date &amp; Time (UTC)</th><th>User ID</th><th>Device ID</th><th>App ID</th><th>Event</th><th>Location</th></tr></thead>
            <tbody>${logRows}</tbody>
          </table>
        </div>` },
    { label: '▲ Access Timeline', content: chart },
    { label: '▦ Top Accessors', content: `
        <div class="two-col">
          <div>
            <div class="section-title">Most Frequent Users</div>
            ${makeBarChart(topU)}
          </div>
          <div>
            <div class="section-title">Most Frequent Devices</div>
            ${makeBarChart(topD, 'var(--orange)')}
          </div>
        </div>` }
  ]);

  el.innerHTML = metrics + tabs + '<hr><div class="section-title">System Anomaly Flags</div>' + anomalyHtml;
}

// ═══════════════════════ MODULE 4 — ANOMALY RADAR ══════════════════════════
function renderAnomalyModules() {
  const modules = [
    ['CRITICAL','BRUTE FORCE DETECTION',    'Continuous FAILED_LOGIN attempts triggering threshold alert.',                         'Planned'],
    ['CRITICAL','MULTI-DEVICE ANOMALY',     'User logs in from N+ devices within a restricted time window.',                        'Planned'],
    ['CRITICAL','GEO-HOPPING ALERT',        'Impossible travel sequence: location jumps exceed flight-time metrics.',               'Planned'],
    ['WARNING', 'OFF-HOURS ACCESS',         'System activity detected outside 08:00–18:00 local business hours.',                   'Planned'],
    ['WARNING', 'SESSION HIJACK RISK',      'ADMIN_ACTION followed by mass DOWNLOAD execution within the same session.',           'Planned'],
    ['WARNING', 'LONG SESSION ANOMALY',     'Session duration exceeds standard operational threshold (> 12h).',                    'Planned'],
    ['WARNING', 'SILENT-THEN-BURST',        'User inactive > 7 days, followed by sudden high-volume event stream.',                'Advanced'],
    ['WARNING', 'CREDENTIAL STUFFING',      'Multiple FAILED_LOGINs from scattered IPs culminating in a successful login.',        'Advanced'],
  ];

  document.getElementById('anomaly-modules').innerHTML = modules.map(([sev,title,desc,tag]) => `
    <div class="alert-box ${sev==='CRITICAL'?'critical':''}">
      <div class="al-header">
        <span class="al-sev ${sev.toLowerCase()}">${sev}</span>
        <span class="al-title">${title}</span>
        <span class="al-tag ${tag.toLowerCase()}">${tag.toUpperCase()}</span>
      </div>
      <div class="al-body">${desc}</div>
    </div>`).join('');
}

// ═══════════════════════ MODULE 5 — DATA QUALITY ═══════════════════════════
async function loadQuality() {
  const el = document.getElementById('quality-result');
  el.innerHTML = loading('Fetching quality report from engine...');
  const [data, err] = await apiFetch('/api/quality');
  if (err) { el.innerHTML = errBox(err); return; }

  const loaded  = data.loaded        || 0;
  const skipped = data.total_skipped || 0;
  const total   = loaded + skipped;
  const pctOk   = total > 0 ? (loaded/total*100).toFixed(1)+'%' : 'N/A';
  const mal = data.malformed || {total:0, samples:[]};
  const inv = data.invalid   || {total:0, samples:[]};
  const dup = data.duplicate || {total:0, samples:[]};

  const metrics = `<div class="metric-row">
    ${metricCard('Total Rows Read', total.toLocaleString(),   'from CSV file')}
    ${metricCard('Loaded',          loaded.toLocaleString(),  'valid rows',         'green')}
    ${metricCard('Skipped',         skipped.toLocaleString(), 'invalid / filtered', skipped>0?'red':'')}
    ${metricCard('Acceptance Rate', pctOk,                    'rows passed validation')}
  </div>`;

  const catCards = `
    <div class="section-title">Skip Category Breakdown</div>
    <div class="cat-row">
      <div class="metric-card red">
        <div class="mc-label">Malformed</div>
        <div class="mc-value">${mal.total.toLocaleString()}</div>
        <div class="mc-sub">Wrong column count</div>
      </div>
      <div class="metric-card orange">
        <div class="mc-label">Invalid</div>
        <div class="mc-value">${inv.total.toLocaleString()}</div>
        <div class="mc-sub">Bad event / location / ID / timestamp</div>
      </div>
      <div class="metric-card">
        <div class="mc-label">Duplicate</div>
        <div class="mc-value">${dup.total.toLocaleString()}</div>
        <div class="mc-sub">Same user + timestamp + event</div>
      </div>
    </div>`;

  function sampleTable(samples, emptyMsg) {
    if (!samples || samples.length === 0) return `<div class="msg success">✓ &nbsp;${emptyMsg}</div>`;
    const rows = samples.map((s,i) => `<tr>
      <td style="color:var(--text2);width:30px">${i+1}</td>
      <td style="font-family:var(--font-mono);font-size:0.72rem;max-width:500px;overflow:hidden;text-overflow:ellipsis" title="${s.line.replace(/"/g,'&quot;')}">${s.line}</td>
      <td style="color:var(--orange)">${s.reason}</td>
    </tr>`).join('');
    return `<div style="font-size:0.65rem;color:var(--text2);margin-bottom:10px;letter-spacing:1px">
      SHOWING ${samples.length} SAMPLE(S) — ENGINE STORES UP TO 300 PER CATEGORY
    </div>
    <div class="data-table-wrap">
      <table class="data-table">
        <thead><tr><th>#</th><th>Raw Row</th><th>Reason</th></tr></thead>
        <tbody>${rows}</tbody>
      </table>
    </div>`;
  }

  const tabs = makeTabs('qual', [
    { label: `⬤ Malformed (${mal.total.toLocaleString()})`, content: sampleTable(mal.samples, 'No malformed rows detected.') },
    { label: `◆ Invalid (${inv.total.toLocaleString()})`,   content: sampleTable(inv.samples, 'No invalid rows detected.') },
    { label: `○ Duplicate (${dup.total.toLocaleString()})`, content: sampleTable(dup.samples, 'No duplicate rows detected.') },
  ]);

  el.innerHTML = metrics + catCards + '<hr>' + tabs;
}

// ═══════════════════════ INIT ════════════════════════════════════════════════
updateEpochDisplay();
initDatasetCount();
renderAnomalyModules();
</script>
</body>
</html>
)HALOHTML";

//  MAIN
int main(int argc, char* argv[]) {
    HaloEngine engine;

    cout << "=================================================" << endl;
    cout << "      HALO CYBER ACCESS ENGINE - API SERVER      " << endl;
    cout << "=================================================" << endl;

    string dataFile = "data/halo_dataset_1_5m.csv";
    if (argc >= 2) {
        dataFile = argv[1];
        cout << "[INFO] Using custom data file: " << dataFile << endl;
    } else {
        cout << "[INFO] Default data path: " << dataFile << endl;
    }

    cout << "[SYSTEM] Loading data..." << endl;
    engine.loadData(dataFile);

    if (engine.pool.total_logs == 0) {
        cout << "[ERROR] Cannot load data from: " << dataFile << endl;
        cout << "[ERROR] Ensure the CSV file exists at the specified path." << endl;
        return 1;
    }

    cout << "[SYSTEM] Loaded " << engine.pool.total_logs << " log entries." << endl;

    Server svr;

    // GET / Serve embedded dashboard
    svr.Get("/", [](const Request&, Response& res) {
        res.set_content(DASHBOARD_HTML, "text/html; charset=utf-8");
    });

    // API 1: Top 10 Resources
    svr.Get("/api/top", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            long long start = 0, end = 9999999999LL;
            if (req.has_param("start") && !req.get_param_value("start").empty())
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty())
                end = stoll(req.get_param_value("end"));
            res.set_content(engine.getTop10ResourcesByTimeJson(start, end), "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error\"}", "application/json");
        }
    });

    // API 2: User Journey
    svr.Get("/api/user", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            long long start = 0, end = 9999999999LL;
            string id = req.has_param("id") ? req.get_param_value("id") : "";
            if (req.has_param("start") && !req.get_param_value("start").empty())
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty())
                end = stoll(req.get_param_value("end"));
            if (id.empty()) { res.set_content("[]", "application/json"); return; }
            res.set_content(engine.getUserJourneyJson(id, start, end), "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content("[]", "application/json");
        }
    });

    // API 3: Resource Tracking
    svr.Get("/api/resource", [&](const Request& req, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            long long start = 0, end = 9999999999LL;
            string id = req.has_param("id") ? req.get_param_value("id") : "";
            if (req.has_param("start") && !req.get_param_value("start").empty())
                start = stoll(req.get_param_value("start"));
            if (req.has_param("end") && !req.get_param_value("end").empty())
                end = stoll(req.get_param_value("end"));
            if (id.empty()) { res.set_content("[]", "application/json"); return; }
            res.set_content(engine.getResourceJourneyJson(id, start, end), "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content("[]", "application/json");
        }
    });

    //API 4: Data Quality 
    svr.Get("/api/quality", [&](const Request&, Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            res.set_content(engine.getQualityJson(), "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content("{\"error\":\"Internal Server Error\"}", "application/json");
        }
    });

    cout << "[SYSTEM] System is ready!" << endl;
    cout << "=================================================" << endl;
    cout << "  Dashboard: http://localhost:8080               " << endl;
    cout << "  Press Ctrl+C to stop                           " << endl;
    cout << "=================================================" << endl;

    #if defined(_WIN32)
        system("start http://localhost:8080");
    #elif defined(__APPLE__)
        system("open http://localhost:8080");
    #elif defined(__linux__)
        system("xdg-open http://localhost:8080");
    #endif

    svr.listen("0.0.0.0", 8080);
    return 0;
}