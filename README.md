# craftifAI_Hackathon_miningGuard_roof
Single ESP32-C3 + HC-SR04 ultrasonic mine roof stability monitor. Detects gradual roof creep vs sudden collapse using baseline + rate-of-change logic, alerts via onboard RGB LED. 
Later this can be communicated via LoRa mesh, the packet hops until it reaches the Anchor (Hub)

Im unable to use LoRa because i dont have the module at the moment.

Also RSSI can be used to detect if there are miners present during collapse.

# Mine Roof Stability Monitor
### FirmGen Hackathon — CraftifAI Buildathon, Aug 8 2026

---

## 1. Problem Statement

Roof falls (collapse of tunnel/gallery ceilings) are the single largest cause of
death in Indian mining — ahead of gas explosions and flooding, per DGMS
(Directorate General of Mines Safety) data. Today, miners judge roof stability
by visual inspection and experience. There is no cheap, continuous, automated
monitoring at most mine faces — existing geotechnical monitoring (extensometers,
tiltmeters) costs lakhs of rupees per unit and requires cabled infrastructure.

## 2. Solution

A single ESP32-C3 + HC-SR04 ultrasonic sensor mounted on a roof bolt/support
pillar, pointed at the tunnel ceiling directly above a work area. It
continuously measures distance to the roof surface and classifies movement
into three states based on **both magnitude and speed of change** — not just
a single threshold.

| Status   | Trigger condition                                              | Onboard RGB LED (GPIO8) |
|----------|-----------------------------------------------------------------|-------------------------|
| SAFE     | Distance within 1cm of baseline                                 | Green, solid            |
| CAUTION  | Distance dropped >2cm from baseline, gradually (slow creep)     | Yellow, slow blink      |
| DANGER   | Distance dropped >2cm from baseline within 1 second (sudden)    | Red, fast blink         |

**Why rate-of-change matters (our key differentiator):** a roof that sags a
few mm over hours is a very different risk profile from a roof that suddenly
drops — the firmware distinguishes gradual creep (early warning, time to
evacuate calmly) from sudden failure (immediate danger, evacuate now). A naive
single-threshold system cannot make this distinction.

## 3. Hardware & Wiring

| Component        | Pin       | Connects to ESP32-C3 |
|-------------------|-----------|------------------------|
| HC-SR04 VCC       | Purple wire | 3V3 (NOT 5V — see note below) |
| HC-SR04 GND       | Black wire  | GND |
| HC-SR04 TRIG      | White wire  | GPIO4 (output) |
| HC-SR04 ECHO      | Grey wire   | GPIO5 (input) |
| Onboard RGB LED   | —           | GPIO8 (WS2812, built into board) |

**Board:** ESP32-C3-DevKitM-1
**Sensor:** HC-SR04 ultrasonic distance sensor
**No buzzer, no OLED, no second device, no LoRa/BLE mesh** — output is via
onboard RGB LED and serial monitor only.

### Known limitation — 3.3V power (documented honestly, not hidden)

HC-SR04 is officially rated for 4.5–5.5V. We power it at 3.3V instead, because
no resistor divider components were available at the event to safely step
down the sensor's normal 5V ECHO output to the ESP32-C3's 3.3V-only GPIO
tolerance. Running at 3.3V keeps ECHO's output voltage safely within GPIO
limits without needing a divider, at the cost of some reading noise/range
reduction versus spec.

**Mitigation:** baseline capture + delta thresholding (rather than raw
absolute-distance thresholds) reduces sensitivity to sensor noise, since we
care about *change* from a known-good reference point, not absolute accuracy.
In a production version, this would use proper level-shifting (10kΩ/20kΩ
resistor divider on ECHO) and full 5V sensor power for rated accuracy.

## 4. Firmware Architecture

- Sensor read task: samples HC-SR04 every 200ms via TRIG pulse / ECHO pulse timing
- Baseline captured once at startup (assumes stable roof position at power-on)
- Delta + rate-of-change logic classifies SAFE / CAUTION / DANGER
- Status drives onboard WS2812 RGB LED (RMT peripheral) and serial UART logging
- Built using ESP-IDF v5.5, generated and refined iteratively via FirmGen's
  Planner → Architect → Coder → Compiler pipeline

## 5. Test Method

1. Power on device with sensor facing a fixed reference surface (~15-25cm away)
2. Confirm SAFE status and green LED once baseline locks in
3. Slowly move a flat rigid object (book/phone) toward sensor over 3-4 seconds
   by 5-10cm → expect CAUTION, yellow LED
4. Reset to baseline distance, wait for SAFE, then move object suddenly
   (<1 second) by similar distance → expect DANGER, red LED
5. Serial monitor prints exact distance, delta, and status for every reading —
   used to verify logic independent of LED visibility

## 6. Scalability & Robustness Notes

- Current build: single standalone node, no networking (by event hardware
  constraint — no LoRa modules or spare boards were available on-site)
- Production roadmap: multiple nodes per mine face reporting over LoRa/SX1262
  mesh to a central monitoring dashboard, exactly the radio-layer swap our
  MeshGuard-style zone-detection design already anticipates
- Sensor fault handling: [confirm with team — add timeout/error status if
  HC-SR04 returns no echo, rather than treating it as a valid reading]

## 7. Cost

Estimated BOM per node: ESP32-C3 (~Rs 150-250) + HC-SR04 (~Rs 60-100) ≈
**under Rs 300-400** per unit, versus lakhs of rupees for existing cabled
geotechnical monitoring systems.

---
*Prepared for CraftifAI FirmGen Hackathon, Bengaluru, Aug 8 2026*



