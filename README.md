# Wet / Dry Waste Sorter

A small bench-top sorter built from:

| Part | What it does |
|------|--------------|
| Arduino Nano | Reads the sensor, drives the servo |
| HW-080 soil moisture sensor (probe + LM393 board) | Measures moisture of the item on the tray |
| SG90 micro servo | Tilts the tray toward the wet or dry bin |
| Breadboard + jumper wires | Connections, plus a "sort now" trigger wire |

## Wiring

```
HW-080 board        Nano
  VCC  ----------->  D7      (switched on only while reading)
  GND  ----------->  GND
  A0   ----------->  A0
  D0   ----------->  (unused)

Probe (2 pins) -----> HW-080 2-pin header  (black + white jumper pair)

SG90 servo
  brown  ---------->  GND
  red    ---------->  5V
  orange ---------->  D9

Trigger
  one jumper in D2, free end touched to GND  (INPUT_PULLUP)
```

Nano is powered over USB. If the Nano resets when the servo moves, put a
100-470 uF electrolytic capacitor across 5V and GND next to the servo, or
power the servo from a separate 5V source with a shared GND.

## Mechanical layout

```
        drop waste here
              |
     +--------v--------+
     |   TRAY (flat)   |   <- probe prongs lie flush in the tray floor
     |  ====probe====  |
     +--------+--------+
              |  servo horn under the centre of the tray
            [SG90]
    /                  \
 WET BIN             DRY BIN
```

- Mount the servo so the horn acts as a pivot under the tray centre. At 90 deg
  the tray is level. At 20 deg it tips left (wet), at 160 deg it tips right (dry).
- Cut two slots in the tray floor so the probe's two metal prongs sit just above
  the surface and the item rests on both. The item must touch both prongs for
  a reading.
- A thin plastic or cardboard tray with a smooth surface lets items slide off
  easily. Tilt angles can be changed in the sketch (`POS_WET`, `POS_DRY`).
- The SG90 can only lift a few tens of grams. Keep the tray light and the items
  small (vegetable peel, tea bag, paper, wrapper).

## Firmware

Sketch: `firmware/waste_sorter/waste_sorter.ino`

1. Install the Arduino IDE. Board: **Arduino Nano**, processor: usually
   **ATmega328P (Old Bootloader)** for clone boards with the CH340 chip.
2. Upload the sketch. Open Serial Monitor at **115200 baud**.

Serial commands:

| Key | Action |
|-----|--------|
| `s` | Run one sort cycle |
| `r` | Toggle live readings every second (for calibration) |
| `t650` | Set wet threshold to 650 at runtime |
| `w` / `d` | Test the servo toward the wet / dry side |

## Calibration

The sensor is resistive: **higher reading = drier**. Open circuit reads about
1023.

1. Send `r` and note the value with an empty tray.
2. Place dry waste (paper, plastic wrapper). Note the value.
3. Place wet waste (banana peel, wet tissue). Note the value.
4. Pick a threshold between the dry and wet numbers and set it with `tNNN`,
   then copy it into `WET_THRESHOLD` in the sketch so it survives a reset.

Typical numbers: empty / dry 950-1023, wet 200-700.

## Limitations and next steps

- Dry items read the same as an empty tray, so the firmware needs a trigger to
  know an item is present. The jumper-to-GND trigger works; a push button or
  an IR obstacle sensor can replace it later.
- The resistive probe corrodes over time. Powering it only while reading (done
  in the sketch) helps. A capacitive probe is a drop-in upgrade.
- Items that do not touch both prongs will be read as dry.
# turbo-happiness
