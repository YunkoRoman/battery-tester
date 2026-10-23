# 4-Channel 18650 Battery Capacity Tester (Arduino / Nanit Pro)

A 4-channel capacity tester for Li-ion (18650) batteries based on the Nanit Pro microcontroller board (ATmega2560).
Current stage: **Building and debugging a 1-channel prototype on a breadboard**.

---

## 1. Bill of Materials

### Power and control section (for 4 channels)
| Component | Specs | Part code (Radiomag) | Quantity |
| :--- | :--- | :--- | :--- |
| **Power resistor** | 10 Ohm, 10W (SQP100JB-10R) | `50513` | 4 pcs |
| **N-MOSFET (Logic-Level)** | IRL540NPBF (TO-220) | `25626` | 4 pcs |
| **Gate protection resistor** | 220 Ohm, 0.25W (CF1/4W-220R) | `92206` | 4 pcs (pack of 30) |
| **Pull-down resistor** | 10 kOhm, 0.25W (MFR025FTB-10KR) | `169934` | 4 pcs (pack of 20) |
| **18650 holder** | KS18650x1-THM (Global Tone) | `151876` | 4 pcs |

### Auxiliary equipment and assembly
* **Breadboard:** 400 or 830 points.
* **Wires:** 0.75 mm² copper wire (for the discharge power path) and Dupont jumpers (for signals).
* **Microcontroller:** Nanit Pro (ATmega2560).

---

## 2. Wiring diagram (1-channel prototype)

### IRL540N MOSFET pinout (pins facing you):
1. **Gate (G)** — left pin
2. **Drain (D)** — middle pin
3. **Source (S)** — right pin

### Electrical connections:
```text
[Nanit Digital Pin D2] ---> [220 Ohm resistor] ---> [Gate IRL540N]
                                                |
                                      [10 kOhm resistor]
                                                |
                                             [GND]

[Battery (+) 4.2V] ---> [10W 10 Ohm ceramic resistor] ---> [Drain IRL540N]
[Battery (+) 4.2V] ---> [Nanit Analog Pin A0] (voltage measurement)

[Battery (-)] ----------------------------------------------> [Source IRL540N] ---> [GND]
```

Important: the microcontroller GND, the battery negative terminal, and the transistor Source must be joined on the blue GND rail of the breadboard.

---

## 3. Math and calculations

Maximum discharge current:

I_max = V_bat / R = 4.2 V / 10 Ω = 0.42 A (420 mA)

Power dissipated in the power resistor:

P = I² × R = (0.42)² × 10 ≈ 1.76 W (the 10W resistor does not overheat)

Power dissipated in the MOSFET:

P_mosfet = I² × R_DS(on) = (0.42)² × 0.04 ≈ 0.007 W (7 mW) → no heatsink needed

Discharge cutoff threshold: 3.0 V (protects the Li-ion cell from deep discharge).

---

## 4. Base code for 1 channel

See `src/battery_tester.ino`.

---

## 5. CLI checklist / Next steps

- [ ] Assemble 1 channel on the breadboard according to the diagram.
- [ ] Upload the base firmware to the Nanit Pro via PlatformIO / Arduino CLI.
- [ ] Verify the voltage reading with a multimeter and calibrate the V_REF factor in the code.
- [ ] Run a full test discharge of one 18650 battery.
- [ ] Scale the code and circuit to 4 channels (adding GATE_PINS[4], VOLTAGE_PINS[4] arrays).
