# Curiosity v0.4 port notes

This firmware was written for the **NASA Perseverance v0.9** main board and has been ported to the
**NASA Curiosity v0.4** main board. The two boards are conceptually the same rover — ESP32-S3-MINI-1,
AW9523B LED expander, XL9555 module expander, TC1508S wheel drivers, MAX98357A speaker amp, a 24-pin
FPC camera and UMAX module ports — so the port is mostly a pinout change. The parts that are *not*
just a pinout change are listed below.

Everything here was derived from the KiCad project at
`NASA Curiosity v0.4` (`.kicad_pcb` net names and footprint pin functions), cross-checked against the
Perseverance v0.9 project.

## ESP32-S3 pin mapping

| Function | Perseverance v0.9 | Curiosity v0.4 |
|---|---|---|
| I2C SDA / SCL (camera, AW9523, XL9555) | 11 / 10 | 11 / 10 |
| Module port I2C SDA / SCL | shared with main bus | **7 / 8** (dedicated, `I2C_NUM_1`) |
| Motor left A / B | 13 / 12 | **14 / 15** |
| Motor right A / B | 15 / 14 | **12 / 13** |
| Servo 1 / 2 / 3 | 16 / 17 / 18 | **18 / 17 / 16** |
| Camera PWDN | 36 | **35** |
| Camera XCLK | 33 | 33 |
| Camera VSYNC / HREF / PCLK | 35 / 34 / 47 | **36** / 34 / 47 |
| Camera D0..D7 | 41,43,44,42,40,39,38,37 | **38,21,20,37,39,40,41,42** |
| I2S BCLK / LRCLK / DOUT | 1 / 3 / 0 | **9 / 3 / 1** |
| Battery ADC | 6 | **none** (see below) |
| Pair button | AW9523 P0_6 | **none** (see below) |
| Power latch (`PWR_HOLD`) | — | **45** |
| Power button (`BTN_SENSE`) | — | **46** |
| Module port IO_1 / IO_2 / IO_6 | 5 / 4 / 2 (port B) | **4 / 2 / 5** |
| USB | native, IO19/IO20 | **CH340C on UART0, IO43/IO44** |

The console was already configured for UART0, so no change was needed there. IO19 now carries the
XL9555 interrupt (unused by the firmware) and IO20 is a camera data line; configuring IO20 as a GPIO
detaches the USB pad from both.

## AW9523B (0x5B) — LED expander

All ten LED channels the firmware uses exist on Curiosity, at different bits, and the board adds
three decorative channels.

| `LED` enum | Net | Perseverance bit | Curiosity bit |
|---|---|---|---|
| `StatusYellow` | `STANDBY_LED` | P1_5 (13) | **P0_0 (0)** |
| `HeadlightsRight` | `HEADLIGHT4` (was `HEADLIGHT1`) | P1_3 (11) | **P0_3 (3)** |
| `Camera` | `CAMERA TOWER LED` | P0_0 (0) | **P0_4 (4)** |
| `HeadlightLeft` | `HEADLIGHT2` | P1_2 (10) | **P0_5 (5)** |
| `Deco2` | `DECO_LED2` | — | **P0_6 (6)** |
| `Deco3` | `DECO_LED3` | — | **P0_7 (7)** |
| `Arm` | `ARM_LED` | P1_1 (9) | **P1_0 (8)** |
| `MotorRight` | `MOTOR_LED1` | P1_0 (8) | **P1_1 (9)** |
| `StatusRed` | `ERROR_LED` | P1_7 (15) | **P1_2 (10)** |
| `StatusGreen` | `GoodToGo_LED` | P1_6 (14) | **P1_3 (11)** |
| `MotorLeft` | `MOTOR_LED2` | P1_4 (12) | P1_4 (12) |
| `Deco1` | `DECO_LED1` | — | **P1_5 (13)** |
| speaker `SD_MODE` | `SPEAKER_SD` | P0_7 (7) | **P1_6 (14)** |
| `Rear` | `STATUS_LEDS` | P0_5 (5) | **P1_7 (15)** |

`Deco1`..`Deco3` were added to the `LED` enum and are switched on in `init()`; they are turned off
along with everything else on shutdown.

## XL9555 (0x20) — module expander

Perseverance used both halves of this expander for its two module ports. Curiosity has one module
port, wired to the same nets Perseverance called port "2" (`ModuleBus::Right`), and repurposes the
freed half for charger and battery status:

| Bit | Curiosity |
|---|---|
| P0_0 (0) | `CHARGE` — TP4056 `CHRG`, active low |
| P0_1 (1) | `STANDBY` — TP4056 `STDBY`, active low |
| P0_2 (2) | `CALIB_EN` — switches the TL431 2.5V reference onto the battery divider |
| P0_3 (3) | `BATTERY_READ` — battery divider tap |
| P0_4 (4) | `USB_DETECT` |
| P0_5..P0_7 (5..7) | module connector IO_3 / IO_4 / IO_5 |
| P1_0..P1_7 (8..15) | module detect and address bits (same layout as Perseverance port "2") |

`Modules` and `Battery` now share one `TCA9555` instance created in `main.cpp`, so the two do not
fight over the direction registers.

## Behavioural changes

**Power latch.** The board latches its own supply: the power button turns the load switch on, and the
rover only stays on while `PWR_HOLD` (IO45) is driven high. `Devices/Power` owns that pin, and
`bootloader_components/init/hooks.c` asserts it in `bootloader_before_init` so the button only has to
be held for a few milliseconds instead of for the whole boot. Powering off means releasing the latch,
which `shutdown()` now does instead of entering deep sleep.

A side effect of the hardware design: any CPU reset (including a panic-triggered restart) resets the
GPIO matrix, drops the latch, and powers the rover off. That is the board's behaviour, not something
the firmware can work around.

**Automatic pairing.** There is no pairing button, so `PairState` starts `PairService` in its
constructor and stays discoverable for as long as it is the active state. The `PairStart` sound is now
queued instead of played with priority, so it no longer cuts off the power-on sound on every boot.

**Power button.** IO46 reads high while the power button is pressed (it hangs off a divider from the
battery). `Input` reports it as `Input::Button::Power`:

- a short press does what the Perseverance pair-button press did in `DriveState` — flips the camera;
- holding it for 1.5 s posts `Input::Data::Hold`, which `PowerButtonService` turns into an orderly
  power off.

Because the rover is turned *on* by holding that same button, `Input` adopts the button's state at
construction and marks the hold as already handled — otherwise the press that powered the rover on
would immediately read as a request to power it off.

**Shutdown path.** The stop-driving / drop-the-link / lights-out sequence was duplicated between
`main.cpp` and `InactivityService`; it is now one `gracefulShutdown()` in `main.cpp`, shared by the
inactivity timeout, the low-battery callback and the power button.

**Camera.** Curiosity uses a **GC2145** sensor instead of the OV2640/OV3660, so
`CONFIG_GC2145_SUPPORT=y` was added to `sdkconfig` (`CONFIG_GC_SENSOR_SUBSAMPLE_MODE` picks itself up
as the Kconfig default, which keeps the wider field of view). No code change was needed: `Feed`
already forces `PIXFORMAT_RGB565` and encodes JPEG in software, which is exactly what the GC2145
needs — it has no hardware JPEG encoder. Detection is by chip ID, so OV2640/OV3660 support can stay
enabled. `XCLK` is still 14 MHz, within the GC2145's 6–27 MHz range, but it is the obvious knob if the
frame rate needs tuning.

**Jig test.** The battery calibration and battery check steps were removed (there is nothing to
calibrate without an ADC) and an XL9555 probe was added. `AudioVisualTest` mutes on the power button.
`ModulesCheck` — still commented out, as it was for Perseverance — was reduced to the single port.

**Hardware version fuse.** `HWVersion::Version` is `0x0104` for Curiosity; Perseverance boards are
fused with `0x0004`, and the mismatch is what stops either firmware from driving the wrong pinout.
Blank boards read `0`; `HWVersion::AllowUnfused` (default `true`) lets those run with a warning so a
fresh board is usable before the jig test fuses it. Set it to `false` to require a fused version.

## Not portable: the battery gauge

**Curiosity v0.4 cannot report battery level, and this firmware reports a full battery and never
triggers the low-battery shutdown.**

The board has the whole measurement circuit — a 1:4 divider off the battery (`R58` 150R, `R60` 300k,
`R61` 100k) plus a TL431 2.5V reference switched in by `CALIB_EN`, which is the same arrangement the
ButterBot firmware calibrates its ADC against. But the divider tap, `BATTERY_READ`, lands on
**XL9555 P0_3, a digital input**, instead of an ADC-capable ESP32 pin. A quarter of a single-cell
voltage (0.75–1.05 V) never crosses the expander's input threshold, so the pin reads low regardless
of charge, and with the reference switched in it reads 0.625 V — also low. There is no reading to be
had, in either position.

Treating that constant low as "battery empty" would shut the rover down at boot, so `Battery` ignores
it. It does sample the tap once at startup and log both positions, so the pin can be checked against
real hardware without reflashing.

What the class still reports is the TP4056 charger state (`Battery::getChargingState()`), from
`CHARGE`, `STANDBY` and `USB_DETECT`. The rover-to-controller protocol has no field for charging
state, so it is only logged.

**Hardware fix:** route `BATTERY_READ` to **IO6**, which is unconnected on v0.4 and is the pin
Perseverance used for exactly this measurement. The divider ratio (4.0) and reference (2500 mV) then
match ButterBot's `Battery` constants directly. Restoring the gauge in firmware after that is
reverting `Devices/Battery` to an `ADCReader` on `PIN_BATT 6` plus a `CALIB_EN` calibration pass.

## Other things to watch on bring-up

- **AW9523B I2C levels.** On Perseverance the expander ran from `BAT+` (3.0–4.2 V); on Curiosity its
  `VCC` is `+5V REGULATED`. Its `VIH` is specified as 0.7 × VDD = 3.5 V, above the ESP32's 3.3 V logic
  high, so the I2C inputs are out of spec on paper. If the AW9523 does not answer at 0x5B, this is the
  first thing to check.
- **Four wheel motors, two channels.** `U2` and `U5` (both TC1508S) share `MOTOR_LEFT_A/B` and
  `MOTOR_RIGHT_A/B`, so each side drives two motors in parallel. No firmware change.
- **Gyro axis flip.** `GyroModule` mirrors X and Y on `ModuleBus::Left`. The single module connector
  is physically on the left side of the rover (`J11` sits on the left edge, next to the left-motor
  silkscreen), which is why the port is reported as `Left` — but the flip is worth confirming against
  a real module.
- **Servo assignment.** `SERVO 1`/`SERVO 2`/`SERVO 3` on the silkscreen match the schematic nets, so
  the firmware's roles carry over unchanged: servo 1 arm position, servo 2 arm pinch, servo 3 camera.
