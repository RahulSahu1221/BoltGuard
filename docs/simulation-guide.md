# BoltGuard — Simulation & Build Guide
### Proteus 8 Professional + LDmicro + Arduino IDE

This is the consolidated, corrected build reference — every wiring and logic decision below reflects the final, working design worked out during development, not the intermediate mistakes along the way.

---

## Table of Contents
1. Software Needed
2. Bill of Materials
3. System Overview
4. Station Controller — Building the Ladder Logic
5. Station Controller — Pin Assignment
6. Sense Controller — Arduino Code
7. Central Monitor
8. Full Wiring Reference
9. The Relay Driver Stage
10. Test Scenarios
11. Known Issues
12. Troubleshooting

---

## 1. Software Needed

| Tool | Purpose |
|---|---|
| Proteus 8 Professional | Circuit simulation |
| Arduino IDE | Compile Sense_Controller.ino to a `.hex` |
| LDmicro | Write and compile the Station Controller's ladder logic |

## 2. Bill of Materials

**Station Controller:** `ATMEGA16`, `BUTTON` ×2 (Start, Reset), `LED-RED`, `LED-GREEN`, `BUZZER`, resistors for LEDs, `BC547` (NPN transistor), `1k` resistor (transistor base), `1N4007` (flyback diode), `RELAY` (12V generic).

**Sense Controller:** `ARDUINO UNO`, `POT-HG` ×4 (torque, angle, drift, LCD contrast), `LM016L` (16×2 LCD), `LED-GREEN`, `LED-RED`, `BUZZER`, resistors.

**Central Monitor:** `VIRTUAL TERMINAL`.

## 3. System Overview

```
[Station Controller: ATmega16]  <--digital I/O-->  [Sense Controller: Arduino Uno]  --serial(TX)-->  [Central Monitor: Virtual Terminal]
```

Build and test each sub-circuit alone before wiring them together.

---

## 4. Station Controller — Building the Ladder Logic

Open LDmicro → `Settings → MCU` → select **ATmega16**. Then `Settings → MCU Parameters` and set **Crystal Frequency to match Proteus exactly** — this is critical; a mismatch here causes constant watchdog resets (see Known Issues). Save the project.

Build these 6 rungs. For any contact that reads back the state of a coil defined elsewhere in the program (rather than a real physical input), double-click it and set its **Source** to **"Output Pin on MCU"** — this makes it correctly read that coil's existing physical pin instead of demanding a separate, unnecessary input pin.

**Rung 1:** `Start` AND NOT `Fault_Lock` → SET `Enable_Bolt1`
**Rung 2:** `Enable_Bolt1` (Source: Output Pin on MCU) AND `Bolt1_Pass` → RESET `Enable_Bolt1`, SET `Enable_Bolt2`
**Rung 3:** `Enable_Bolt2` (Source: Output Pin on MCU) AND `Bolt2_Pass` → RESET `Enable_Bolt2`, SET `Enable_Bolt3`
**Rung 4:** `Enable_Bolt3` (Source: Output Pin on MCU) AND `Bolt3_Pass` → RESET `Enable_Bolt3`, SET `Line_Advance`
**Rung 5:** `Any_Fail` → SET `Fault_Lock`, RESET `Enable_Bolt1`, RESET `Enable_Bolt2`, RESET `Enable_Bolt3`, RESET `Line_Advance`
**Rung 6:** `Reset` AND `Fault_Lock` (Source: Output Pin on MCU) → RESET `Fault_Lock`

Note: `Fault_Lock` appears as a contact in both Rung 1 (normally closed) and Rung 6 (normally open) — set the Source correctly on both occurrences.

`Compile → Compile`. On success this produces a `.hex` file to load into the ATmega16 in Proteus.

## 5. Station Controller — Pin Assignment

| Signal | Pin | Type |
|---|---|---|
| Start | PA0 | Input |
| Reset | PA1 | Input |
| Bolt1_Pass | PA2 | Input |
| Bolt2_Pass | PA3 | Input |
| Bolt3_Pass | PA4 | Input |
| Any_Fail | PA5 | Input |
| Enable_Bolt1 | PB0 | Output |
| Enable_Bolt2 | PB1 | Output |
| Enable_Bolt3 | PB2 | Output |
| Line_Advance | PB3 | Output |
| Fault_Lock | PB4 | Output |

Only these 11 physical signals need a pin. Nothing else should show up requiring assignment — if it does, its contact Source is probably still set to "Input Pin" instead of "Output Pin on MCU."

---

## 6. Sense Controller — Arduino Code

Key structure of `Sense_Controller.ino`:

- Reads torque (`A0`), angle (`A1`), and drift (`A2`) potentiometers.
- Reads which bolt station is currently active from the Station Controller (digital inputs).
- Compares torque and angle against a pass window; pulses a PASS or FAIL signal back to the Station Controller.
- On PASS, stores that bolt's angle as a baseline for Stage 2.
- When no station is active (all bolts already passed), enters watch mode: periodically re-reads angle and compares to the stored baseline, sending a `Serial.println("ALERT: Bolt N drifted X units - check joint")` message when drift exceeds the threshold.
- Drives the LCD, pass/fail LEDs, and buzzer throughout.

Two wiring rules that matter for this code to behave correctly:
- **LCD pin 5 (RW) must be tied directly to GND** — if it's left floating, the LCD will never display anything, including the startup message, no matter what the code sends it.
- Keep the LCD's data pins and any other digital signal on entirely separate physical pins — never let two different signals share one Arduino pin.

## 7. Central Monitor

Wire the Arduino's **TX (pin 1)** to the Virtual Terminal's **RXD**, and Arduino **GND** to Virtual Terminal **GND**. Set the Virtual Terminal's Baud Rate to **9600**, matching `Serial.begin(9600)` in the code. The Virtual Terminal has no GND pin of its own — that's expected, not a missing connection.

---

## 8. Full Wiring Reference

**Station Controller ↔ Sense Controller link:**

| ATmega16 Pin | Arduino Pin | Direction |
|---|---|---|
| PB0 | (Sense Controller input) | ATmega16 → Arduino |
| PB1 | (Sense Controller input) | ATmega16 → Arduino |
| PB2 | (Sense Controller input) | ATmega16 → Arduino |
| PA2 | (Sense Controller output) | Arduino → ATmega16 |
| PA3 | (Sense Controller output) | Arduino → ATmega16 |
| PA4 | (Sense Controller output) | Arduino → ATmega16 |
| PA5 | (Sense Controller output) | Arduino → ATmega16 |

Share a common GND between both controllers. Always verify the exact Arduino-side pin numbers against what's actually defined in `Sense_Controller.ino` — the specific pin numbers matter less than the two sides matching each other exactly.

## 9. The Relay Driver Stage

A digital output pin cannot drive a relay coil directly — it doesn't supply enough current, and the coil needs its own power source. The correct stage:

- `Line_Advance` (PB3) → base resistor (1kΩ) → **Q1 (BC547) Base**
- **Q1 Emitter** → GND
- **Q1 Collector** → one leg of the relay coil (call it the "collector side")
- The relay coil's other leg ("+5V side") → +5V
- **D2 (1N4007)** sits directly across those same two coil legs: cathode (striped end) on the +5V side, anode on the collector side — this absorbs the voltage spike when the transistor switches off, protecting Q1.
- The relay's separate switch contacts (mechanically linked to the coil, electrically independent) don't need to be wired to anything for this simulation.

When `Line_Advance` goes HIGH, Q1 turns on, current flows from +5V through the coil to ground through Q1, and the relay energizes.

## 10. Test Scenarios

**All Pass:** Set torque and angle within the pass window for all three bolts in sequence. Watch the sequence advance Bolt 1 → 2 → 3, ending with Line_Advance / the relay energizing.

**A Fault:** Set one bolt's torque or angle outside the pass window. Confirm the station locks — fault LED and buzzer stay on — and that pressing Start again does nothing until Reset is pressed.

**Loosening Alert:** After all three bolts pass, slowly adjust the drift potentiometer. Confirm the Virtual Terminal prints an alert once drift crosses the threshold.

## 11. Known Issues

As of the current build, an intermittent AVR watchdog reset loop on the Station Controller (ATmega16) is under active investigation — it can prevent the sequence from advancing past Bolt 1. Confirmed **not** caused by a crystal frequency mismatch (frequency was verified matching between LDmicro and Proteus with 0% timing deviation). Currently being isolated by testing an alternate internal timer (Timer0 instead of Timer1) and a longer PLC cycle time. This section will be updated once resolved.

## 12. Troubleshooting

| Problem | Likely Cause | Fix |
|---|---|---|
| LCD shows nothing at all, ever | RW pin floating instead of tied to GND | Wire LCD pin 5 to GND |
| LCD shows blank or solid blocks | Contrast potentiometer maladjusted | Adjust the contrast POT-HG while the simulation runs |
| Compile error: "'X...' is not assigned" | A readback contact's Source is set to "Input Pin" instead of "Output Pin on MCU" | Double-click the contact, change its Source, recompile |
| Constant AVR WATCHDOG reset messages in the Simulation Log | Most likely a genuine logic/timing issue rather than frequency (see Known Issues) — frequency mismatch was already ruled out in this build | Try Timer0 instead of Timer1 in LDmicro's PLC Configuration, and/or increase Cycle Time |
| Relay never energizes | Collector wired directly to +5V instead of to the coil, or coil's other leg not connected to +5V | Recheck the exact node structure in Section 9 |
| Station never advances past Bolt 1 | Related to the watchdog reset issue above, or a genuine wiring gap between the two controllers | Cross-check Section 8's pin table against the actual code and physical wiring |
