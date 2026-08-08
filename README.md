<div align='center'>

# BoltGuard

**A Two-Stage Fastening Verification & Bolt-Loosening Early-Warning System**

BoltGuard is a simulated embedded system that verifies bolted joints at the moment of installation *and* continues watching them afterward for signs of vibration-induced loosening — closing a gap that most factory-floor fastening QC systems leave open.

<br>

<img src='https://img.shields.io/badge/Status-System%20Concept%20Finalized-4caf50?style=for-the-badge' /> <img src='https://img.shields.io/badge/Architecture-Two--Stage%20Design%20Complete-7b1fa2?style=for-the-badge' /> <img src='https://img.shields.io/badge/Firmware-Sense%20Controller%20Ready-1976d2?style=for-the-badge' /> 
<img src='https://img.shields.io/badge/Ladder%20Logic-LDmicro%20Pending-455a64?style=for-the-badge' /> <img src='https://img.shields.io/badge/Central%20Monitor-Implementation%20Pending-546e7a?style=for-the-badge' /> <img src='https://img.shields.io/badge/Simulation-Proteus%20Pending-f57c00?style=for-the-badge' /> <img src='https://img.shields.io/badge/Communication-UART%20Alert%20Link-00897b?style=for-the-badge' />
<img src='https://img.shields.io/badge/Controller-ATmega16%20%2B%20Arduino%20Uno-1565c0?style=for-the-badge' /> <img src='https://img.shields.io/badge/Verification-Torque%20%2B%20Angle%20Monitoring-ef6c00?style=for-the-badge' /> <img src='https://img.shields.io/badge/Domain-Industrial%20Automation-546e7a?style=for-the-badge' /> 

</div>

---

## Table of Contents
- [Overview](#overview)
- [The Problem](#the-problem)
- [The Approach — Two Stages](#the-approach--two-stages)
- [System Architecture](#system-architecture)
- [How It Works](#how-it-works)
- [Tech Stack](#tech-stack)
- [Repository Structure](#repository-structure)
- [Current Status](#current-status)
- [Roadmap](#roadmap)
- [Limitations](#limitations)

---

## Overview

Most automated fastening lines check a bolt exactly once: at the moment it's tightened. If torque and angle look correct, the bolt is marked "pass" and never checked again. BoltGuard extends that single checkpoint into an ongoing verification pipeline — it confirms correct installation **and** keeps monitoring the same joint afterward, so early signs of loosening can be caught before the joint actually fails.

The system is built around two cooperating microcontroller units and is currently developed and tested as a Proteus circuit simulation, with no physical hardware required to demonstrate the concept.

## The Problem

A bolted joint fails in one of two fundamentally different ways:

1. **Installation error** — under-torque, over-torque, cross-threading, or a skipped fastener. This happens during assembly and is what conventional torque/angle QC already targets.
2. **In-service self-loosening** — a correctly installed bolt can still back off gradually over time due to continuous vibration (a well-documented mechanical phenomenon in engineering literature, often referred to as vibration-induced self-loosening). This failure mode is invisible to standard install-time inspection because it only develops *after* the part has already passed QC and left the line.

BoltGuard treats these as two separate problems that need two separate checks, rather than assuming a single pass/fail moment is enough.

## The Approach — Two Stages

**Stage 1 — Install Verification**
While a bolt is being fastened, the system reads torque and rotation angle and checks both against an acceptable pass window before allowing the next bolt in sequence to begin. If a bolt fails, the whole station locks and requires a manual reset — a mistake-proofing (Poka-Yoke) behavior that prevents a bad fastener from silently moving down the line.

**Stage 2 — In-Service Watch**
Once a bolt passes, its final angle is stored as a baseline. The system then periodically re-samples that bolt's angular position. A meaningful backward rotation — the earliest mechanical signature of loosening — triggers an early-warning alert, long before the joint would visibly fail or a technician would notice by inspection.

## System Architecture

```
                    ┌─────────────────────────────┐
                    │      STATION CONTROLLER     │
                    │   (ATmega16 + ladder logic) │
                    │                             │
                    │  Enforces bolt sequence,    │
                    │  locks on any FAIL,         │
                    │  requires manual RESET      │
                    └─────────────┬───────────────┘
                                  │ digital I/O
                                  ▼
                    ┌─────────────────────────────┐
                    │       SENSE CONTROLLER      │
                    │        (Arduino Uno)        │
                    │                             │
                    │  Stage 1: torque + angle    │
                    │  pass/fail check per bolt   │
                    │                             │
                    │  Stage 2: angle drift watch │
                    │  after installation         │
                    └─────────────┬───────────────┘
                                  │ serial link
                                  ▼
                    ┌─────────────────────────────┐
                    │      CENTRAL MONITOR        │
                    │  Displays live station      │
                    │  status + loosening alerts  │
                    └─────────────────────────────┘
```

Torque and rotation angle are represented in simulation using analog potentiometers, standing in for real torque and angle transducers. The alert path between the Sense Controller and the Central Monitor is a serial (UART) link, standing in for what would be a wireless link in a physical deployment.

## How It Works

1. An operator (or automated trigger) starts the sequence; only the first bolt's station is active.
2. The Sense Controller reads that bolt's torque and angle and reports pass or fail to the Station Controller.
3. On pass, the Station Controller disables the current station and enables the next — a bolt cannot be skipped or done out of order.
4. On fail, the Station Controller immediately locks all stations, keeps a fault indicator active, and will not resume until a manual reset is triggered — preventing an operator from pushing a bad fastener through.
5. Once all bolts in the sequence pass, each one's final angle is stored as a baseline and the system enters watch mode.
6. In watch mode, the Sense Controller periodically re-checks angle against the stored baseline for each installed bolt. A drift beyond a defined threshold raises an alert, sent to the Central Monitor with the affected bolt's identifier.

## Tech Stack

| Component | Role |
|---|---|
| **ATmega16 + LDmicro** | PLC-style ladder logic — bolt sequencing, fault interlock, latching reset behavior |
| **Arduino Uno (C/C++)** | Sensor reading, pass/fail evaluation, drift monitoring, LCD/LED/buzzer status output |
| **Proteus 8 Professional** | Full circuit simulation and validation environment |
| **Potentiometers** | Simulated torque, angle, and vibration-drift sensors |
| **16×2 LCD** | Local status display |
| **UART Serial** | Alert reporting to the Central Monitor |

## Repository Structure

```
BoltGuard/
├── README.md
├── docs/
│   ├── project-idea.md          # detailed concept write-up
│   └── simulation-guide.md      # detailed build/wiring guide
├── firmware/
│   ├── Sense_Controller.ino     # Arduino sketch (Stage 1 + Stage 2 logic)
│   └── station_controller/      # LDmicro ladder logic project (pending)
└── presentation/
    └── BoltGuard.pptx
```

## Current Status

- [x] Problem definition and two-stage concept finalized
- [x] System architecture designed
- [x] Sense Controller firmware written (`Sense_Controller.ino`) — torque/angle pass-fail check and angle drift watch logic implemented
- [ ] Station Controller ladder logic (PLC sequencing and fault interlock) — in progress
- [ ] Central Monitor implementation
- [ ] Full Proteus circuit build and simulation
- [ ] End-to-end testing across pass, fault, and drift-alert scenarios

## Roadmap

- Build and validate the Station Controller ladder logic in LDmicro
- Wire and simulate the complete circuit in Proteus
- Run and document the three core test scenarios: all-pass, fault-and-reset, and loosening-alert
- Explore a physical hardware prototype using real torque/angle sensing hardware
- Explore wireless alerting (e.g. RF or IoT module) as a replacement for the current wired serial link

## Limitations

BoltGuard is currently a **concept simulation**, not a certified industrial monitoring device:
- Torque and angle values are simulated using potentiometers, not real transducers
- The alert path is a wired serial link in simulation, standing in for a real wireless link
- Loosening drift in Stage 2 is manually simulated rather than caused by real vibration

These are deliberate simplifications made to validate the system logic and architecture before investing in physical sensing hardware.
