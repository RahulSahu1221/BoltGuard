<div align="center">

# BoltGuard
### *Verified at Install. Watched for Life.*

A **simulation-based industrial fastening prototype** that verifies bolt torque and angle at the moment of installation, then continues watching each bolt afterward for early signs of vibration-induced loosening — extending fastening quality control from a one-time install-time check into an ongoing joint-health system.

Built and validated in **Proteus 8 Professional**, using an **ATmega16** running PLC ladder logic (via LDmicro) alongside an **Arduino Uno** as the sensing and decision controller.

---

![Arduino](https://img.shields.io/badge/ARDUINO-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![Proteus](https://img.shields.io/badge/PROTEUS%208%20PROFESSIONAL-E4A11B?style=for-the-badge)
![PLC Ladder Logic](https://img.shields.io/badge/PLC%20LADDER%20LOGIC-D32F2F?style=for-the-badge)
![Simulation](https://img.shields.io/badge/SIMULATION%20BASED-4CAF50?style=for-the-badge)
![Poka Yoke](https://img.shields.io/badge/POKA--YOKE%20INTERLOCK-2196F3?style=for-the-badge)
![Predictive Maintenance](https://img.shields.io/badge/PREDICTIVE%20MAINTENANCE-5A0FC8?style=for-the-badge)
![Status](https://img.shields.io/badge/STATUS-SIMULATION%20IN%20PROGRESS-BA7517?style=for-the-badge)
![License](https://img.shields.io/badge/LICENSE-MIT-8BC34A?style=for-the-badge)

---
**If you find this project useful, consider giving it a star!**
</div>

---

## Table of Contents
- [Overview](#overview)
- [Project Status](#project-status)
- [The Problem](#the-problem)
- [The Solution](#the-solution)
- [Key Features](#key-features)
- [Engineering Principle](#engineering-principle)
- [System Architecture](#system-architecture)
- [Components Used](#components-used)
- [Circuit & Wiring](#circuit--wiring)
- [Logic Flow](#logic-flow)
- [Simulation Test Results](#simulation-test-results)
- [Technologies Used](#technologies-used)
- [Repository Structure](#repository-structure)
- [Running the Simulation](#running-the-simulation)
- [Honest Limitations & Future Scope](#honest-limitations--future-scope)
- [License](#license)

---

## Overview

Most automated fastening lines check a bolt exactly once: at the moment it's tightened. If torque and angle look correct, the bolt is marked "pass" and never inspected again. BoltGuard treats that as only half the job. It verifies torque and angle at install, using a Poka-Yoke (mistake-proofing) sequence that won't let a station advance past a failed bolt — and for every bolt that passes, it keeps sampling that bolt's angle afterward, so a meaningful backward drift (the earliest mechanical signature of loosening) raises an alert before the joint actually fails.

## Project Status

This project is at the **active simulation and debugging stage**. The PLC ladder logic (Station Controller) is written and compiling cleanly in LDmicro, the sensing/decision firmware (Sense Controller) is written and running on the Arduino, and the full Proteus circuit — sensors, LCD, indicators, relay driver stage, and serial alert link to a Central Monitor — is wired end to end. Stage 2 (the loosening-drift alert) has already been confirmed working in simulation, correctly reporting a drifted bolt over serial. Stage 1's full three-bolt sequence is currently blocked by an intermittent watchdog-reset issue being actively isolated and fixed (see [Honest Limitations & Future Scope](#honest-limitations--future-scope)). No physical hardware has been built — this is a simulation-first prototype, by design, so the logic and safety behavior are fully validated before any physical components are committed to.

## The Problem

A bolted joint fails in one of two very different ways: an installation error (under-torque, over-torque, cross-threading, a skipped fastener) caught by standard QC, or in-service self-loosening — a correctly installed bolt slowly backing off over time due to continuous vibration, a well-documented mechanical phenomenon that standard install-time inspection is structurally blind to, because it only happens after the part has already left the line with a "pass" on record.

## The Solution

BoltGuard splits a bolt's life into two watched phases:

- **Stage 1 — Install Verification:** Torque and final angle are checked against a pass window for each bolt in sequence. A failed bolt locks the entire station — no advancing to the next bolt, no silent skipping — until a deliberate manual reset.
- **Stage 2 — In-Service Watch:** Every bolt that passes has its angle stored as a baseline. The system keeps re-sampling that angle; a significant backward rotation triggers an early-warning alert, reported to a Central Monitor, before the joint is actually loose.

## Key Features

- Sequential multi-bolt Poka-Yoke interlock with forced-restart-on-fault behavior
- Dedicated PLC ladder logic layer (SET/RESET latching) separate from the sensing/decision layer
- Post-install angular drift monitoring — a genuinely underused idea in standard fastening QC
- Live LCD status display, pass/fail indicators, and audible fault alerts
- Serial-based alert reporting to a separate Central Monitor
- Fully simulated, reproducible, and testable without physical hardware

## Engineering Principle

Every design choice follows one rule: **a bolt's job isn't done the moment it's tight — it's done when it stays tight.** Verification and monitoring are treated as two separate responsibilities, handled by two separate controllers, so each can be reasoned about, tested, and debugged independently.

## System Architecture

```
                    ┌───────────────────────────┐
                    │      STATION CONTROLLER   │
                    │   ATmega16 + LDmicro      │
                    │   ladder logic            │
                    │                           │
                    │  Enforces bolt sequence,  │
                    │  locks on any FAIL,       │
                    │  requires manual RESET    │
                    └─────────────┬─────────────┘
                                  │ digital I/O
                                  ▼
                    ┌──────────────────────────────┐
                    │       SENSE CONTROLLER       │
                    │        Arduino Uno           │
                    │                              │
                    │  Stage 1: torque + angle     │
                    │  pass/fail check per bolt    │
                    │                              │
                    │  Stage 2: angle drift watch  │
                    │  after installation          │
                    └─────────────┬────────────────┘
                                  │ serial link
                                  ▼
                    ┌──────────────────────────────┐
                    │      CENTRAL MONITOR         │
                    │  Displays live status +      │
                    │  loosening alerts by bolt ID │
                    └──────────────────────────────┘
```

## Components Used

- ATmega16 microcontroller (Station Controller)
- Arduino Uno (Sense Controller)
- 16×2 LCD (LM016L) with contrast potentiometer
- Potentiometers simulating torque, angle, and vibration-drift sensors
- Push buttons (Start, Manual Reset)
- LEDs and buzzers for pass/fail/fault indication
- BC547 NPN transistor + 1N4007 flyback diode + relay (Line Advance driver stage)
- Virtual Terminal (Central Monitor)

## Circuit & Wiring

The full schematic connects the Station Controller and Sense Controller through a dedicated set of digital lines carrying "which bolt is active" and "pass/fail" signals in both directions, a shared ground, sensor inputs on the Sense Controller's analog pins, an LCD driven in 4-bit mode, and a transistor-driven relay stage that energizes only once all three bolts have passed. Full pin-by-pin wiring detail is maintained in `/docs/simulation-guide.md`.

<img width="2468" height="1275" alt="Wiring Circuit" src="https://github.com/user-attachments/assets/41521fc3-2d7f-42cf-860b-878419171d60" />


## Logic Flow

1. Operator presses Start → Bolt 1's station activates.
2. Sense Controller checks torque + angle → reports pass or fail.
3. On pass, Station Controller disables the current station and enables the next.
4. On fail, Station Controller locks all stations and holds the fault until Reset is pressed.
5. Once all three bolts pass, each bolt's angle is stored as a baseline and the system enters watch mode.
6. Sense Controller periodically re-checks each installed bolt's angle; a drift past threshold sends an alert to the Central Monitor.

## Simulation Test Results

- **Stage 2 (drift alert):** confirmed working — a manually simulated drift on an installed bolt correctly produced a serial alert ("Bolt 1 drifted N units — check joint") on the Central Monitor.
- **Stage 1 (multi-bolt sequence):** blocked by an intermittent watchdog-reset issue currently under active investigation; full pass-through of Bolt 2 and Bolt 3 has not yet been confirmed.
- **LCD status display:** not yet confirmed operational — likely linked to the same reset issue above, to be re-verified once resolved.

## Technologies Used

| Tool | Role |
|---|---|
| LDmicro | PLC ladder logic authoring and AVR compilation |
| Arduino IDE | Sense Controller firmware |
| Proteus 8 Professional | Full circuit simulation and validation |

## Repository Structure

```
BoltGuard/
├── README.md
├── docs/
│   ├── BoltGuard.pdf
│   ├── project-idea.md
│   └── simulation-guide.md
├── firmware/
│   ├── Sense_Controller.ino
│   └── BoltGuard_Station_Controller.ld
├── Boltguard_simulation.pdsprj
```

## Running the Simulation

1. Open `station_controller` in LDmicro and compile to `.hex`.
2. Open `Sense_Controller.ino` in the Arduino IDE and export a compiled binary.
3. Open `Boltguard_simulation.pdsprj` in Proteus 8 Professional, load both `.hex` files onto their respective components, and run.

## Honest Limitations & Future Scope

- Torque, angle, and vibration-drift are simulated using potentiometers, not real transducers.
- The alert path to the Central Monitor is a wired serial link in simulation, standing in for a real wireless link in a physical build.
- A watchdog-reset issue is currently blocking full validation of the three-bolt sequence and is being actively debugged.
- No physical hardware has been built yet; that remains a planned future phase once the simulation is fully validated.

## License

MIT
