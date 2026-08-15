# BoltGuard — Project Idea

## Table of Contents
1. What Is BoltGuard (One-Minute Summary)
2. The Real-World Problem
3. The Gap in Current Industry Practice
4. How BoltGuard Solves It
5. System Architecture (High Level)
6. Full Workflow — A Bolt's Journey Through BoltGuard
7. Engineering Background Behind the Design
8. Honest Limitations
9. Where to Go Next

---

## 1. What Is BoltGuard (One-Minute Summary)

BoltGuard is a **two-stage bolt fastening safety system** built for automated production lines.

- **Stage 1 — Install Verification:** While a bolt is being fastened, BoltGuard checks that it reaches the *correct torque* AND the *correct final angle* before allowing the line to move to the next bolt. If any bolt fails, the whole station locks and refuses to continue until a human resets it — a Poka-Yoke (mistake-proofing) behavior.
- **Stage 2 — In-Service Watch:** After a bolt is installed and passed, BoltGuard doesn't stop watching. It keeps monitoring that bolt's angular position over time. If the bolt starts rotating back — the earliest sign of vibration-induced loosening — it raises an alert **before** the joint actually becomes loose.

In short: most fastening QC systems only ask *"did we tighten it correctly?"* BoltGuard also asks *"is it staying tight?"*

## 2. The Real-World Problem

A bolted joint can fail in two very different ways:

1. **Installation error** — under-torqued, over-torqued, cross-threaded, or simply skipped. This is what most factory QC already tries to catch.
2. **In-service self-loosening** — even a *perfectly* installed bolt can slowly back off over weeks or months because of continuous vibration. This is a well-documented mechanical phenomenon, and it happens *after* the part has already left the factory floor with a "PASS" already on record.

## 3. The Gap in Current Industry Practice

Standard torque/angle verification systems stop working the instant a bolt passes inspection. From that point on, the bolt is invisible to the factory's quality system. If it loosens later, nobody finds out until there's a rattle, a leak, or a failure. BoltGuard closes that gap by extending fastening verification from a single install-time snapshot into an ongoing health check.

## 4. How BoltGuard Solves It

Two cooperating controllers:

- **Station Controller** (PLC ladder logic) — the gatekeeper. Enforces the correct bolt-by-bolt sequence during installation and refuses to let the line move forward until every bolt passes.
- **Sense Controller** (Arduino) — the inspector and watchman. During Stage 1 it reads torque and angle sensors and reports pass/fail per bolt. During Stage 2, for bolts that already passed, it keeps sampling angular position and compares it against the angle recorded right after installation, alerting on meaningful backward drift.

## 5. System Architecture (High Level)

```
                    ┌───────────────────────────┐
                    │      STATION CONTROLLER   │
                    │   (ATmega16 + LDmicro     │
                    │    ladder logic)          │
                    │                           │
                    │  Enforces bolt sequence:  │
                    │  Bolt1 → Bolt2 → Bolt3    │
                    │  Locks on any FAIL        │
                    │  Requires manual RESET    │
                    └─────────────┬─────────────┘
                                  │
                                  ▼
                    ┌─────────────────────────────┐
                    │       SENSE CONTROLLER      │
                    │        (Arduino Uno)        │
                    │                             │
                    │  STAGE 1: reads torque +    │
                    │  angle sensors, checks      │
                    │  against pass window        │
                    │                             │
                    │  STAGE 2: after PASS, keeps │
                    │  sampling angle over time,  │
                    │  flags backward drift       │
                    └─────────────┬───────────────┘
                                  │
                                  ▼
                     ┌──────────────────────────┐
                     │      CENTRAL MONITOR     │
                     │  Displays live station   │
                     │  status + any loosening  │
                     │  alerts by Bolt ID       │
                     └──────────────────────────┘
```

Electrical building blocks: potentiometers simulating torque/angle/drift sensors, a 16×2 LCD for status, LEDs and buzzers for Good/No-Good indication, push buttons for Start and Manual Reset, a transistor-driven relay stage representing "line may now advance," and a serial link carrying loosening alerts to the Central Monitor.

## 6. Full Workflow — A Bolt's Journey Through BoltGuard

1. Operator presses Start. Bolt 1's station activates.
2. Sense Controller reads Bolt 1's torque and angle. If both are inside the pass window, it reports PASS.
3. Station Controller disables Bolt 1's station and enables Bolt 2's — Bolt 2 cannot start before this happens.
4. This repeats through Bolt 3. Once all three pass, the line may advance.
5. If any bolt fails, the Station Controller immediately locks all stations until a manual reset is pressed — preventing a bad fastener from silently moving down the line.
6. Every bolt that passed has its final angle stored as a baseline.
7. In watch mode, the Sense Controller periodically re-checks each installed bolt's angle against its baseline.
8. A meaningful backward drift triggers an alert to the Central Monitor, naming the affected bolt.

## 7. Engineering Background Behind the Design

Every block in BoltGuard maps onto standard fundamentals: sensors and transducers (potentiometers standing in for torque/angle sensors), analog and digital electronics (ADC reading, threshold comparison), control systems basics (sequencing, feedback loops), digital logic and PLC fundamentals (ladder logic rungs, SET/RESET latching), power electronics (transistor-driven relay switching, flyback protection), and serial communication (the alert reporting path).

## 8. Honest Limitations

- This is a concept simulation, not a certified industrial monitoring device.
- Torque, angle, and drift are simulated using potentiometers, not real transducers.
- The alert path is a wired serial link in simulation, standing in for a real wireless link in a physical deployment.
- Loosening drift in Stage 2 is manually simulated rather than caused by real vibration.

## 9. Where to Go Next

For the full parts list, wiring diagrams, ladder logic build steps, and complete Arduino code, see the companion file: **`simulation-guide.md`**
