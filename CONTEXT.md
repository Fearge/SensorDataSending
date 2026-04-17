# Project Context

## Goal
- Loadcell Sensor data collection with HX71708 ADCs
- used for 4-Day excibition, pure data patch uses sensor data to produce sound
- People stand on plate with load cells to control pure data patch with body weight
- Same Experience for all people, with all different weights
- Current runtime target: Arduino Nano ESP32
- Future plan: ESP32 master with Nano sensor nodes over RS485
- Change Data Output to SLIPOsc

## Target Architecture
- Sensor layer: read four HX71708 channels and keep raw values separate from processed values
- Signal layer: build two pair means, derive one signed balance value from them, and treat total load only as a plausibility signal
- Output layer: send one compact control value to Pure Data, optionally plus a presence/idle indicator
- Transport layer: replace OSC with SLIPOsc for the current setup, then replace only the transport with RS485 later
- Stability layer: tare only when the platform is clearly unloaded, add startup settling time, and avoid automatic re-zeroing while a person is standing on the platform

## Current State
- Current branch: weiterentwicklung
- Main source entry: src/main.cpp
- Central app constants: src/app_config.h
- Runtime helper module: src/sensor_runtime.cpp
- Signal helper module: src/signal_processing.cpp
- Transport helper module: src/transport_osc.cpp
- ADC driver: src/HX71708_ADC.cpp
- Current output: 1 signed balance value in range -512...512
- WiFi AP and OSC are still active in the current build
- RS485 bus prototype exists in src/bus.cpp but is not wired into main yet

## Hardware
- Board(s): Arduino Nano ESP32 master, multiple Arduino Nano V3's as Nodes
- Sensor type: HX71708-based load cell ADCs
- RS485 transceiver: planned, not yet integrated
- Power supply: not documented yet
- Important pins: SCK pins 3, 5, 7, 9; DOUT pins 2, 4, 6, 8

## Data Flow
- Sensor raw values: read from four HX71708 ADCs in src/HX71708_ADC.cpp
- Tare/zero handling:
- startup warmup reads before tare are active via src/sensor_runtime.cpp
- short settle delay before each sensor tare is active via src/sensor_runtime.cpp
- tare() now uses block averaging with trimmed extremes in src/HX71708_ADC.cpp
- Scaling/unit: scale factors are configured in src/app_config.h and applied in src/sensor_runtime.cpp
- Output format: OSC message with a single signed balance value
- Balance interpretation: -512 means full load on one sensor pair, 0 means even load, +512 means full load on the other pair
- Load handling: absolute load should not change the interaction feel, only the balance and presence should matter

## Communication Protocol
- Bus type: OSC over WiFi for the current implementation, later SLIPOsc
- Frame format: not yet finalized for RS485
- Byte order: planned little-endian for fixed-width bus payloads
- CRC/checksum: planned for RS485 frames
- Timing/slot rules: planned master sync with fixed slots

## Known Issues
- Tare can be unstable if the load is still settling at startup
- sometimes doesn't correctly or sensors time out
- Balance and presence thresholds still need practical tuning with real users
- transport and signal logic are separated, but module boundaries can still be improved

## Decisions
- Output should be a single balance value in the range -512..512
- Use fixed-width integer types for bus payloads later
- Keep RS485 protocol compact and CRC protected

## Implementation Roadmap
1. Make the sensor output stable before touching transport [done]
- add a short startup settle phase before tare [done]
- add a timeout to read320Hz() [done]
- keep the current OSC output, but only send one balance value [done]

2. Simplify the signal path in main.cpp [done]
- compute one balance value from the two sensor pairs [done]
- use total load only to decide whether a person is present [done]
- remove leftover normalization code that is no longer needed [done]

3. Keep the data model aligned with the exhibition goal
- make the output independent of absolute body weight
- map equal load to 0 and shifted load to negative/positive extremes
-  emit a presence flag later if Pure Data needs it

4. Prepare the code for the later RS485 transition
- keep protocol details in a separate transport layer
- use fixed-width integer types for all future bus payloads
- define frame format, CRC, and timing only after the signal behavior is stable

5. Reduce hidden coupling between modules
- keep HX71708 driver focused on raw read, tare, and calibration
- keep main.cpp as orchestration and signal shaping only [done]
- avoid reintroducing transport logic into the sensor layer

## Open Questions
- Which RS485 transceiver and baud rate will be used?

## TODO
- Tune PRESENCE_THRESHOLD and balance sensitivity with real test users
- Decide final RS485 frame layout
- Move from OSC to SLIP OSC output path
- Continue reducing src/main.cpp to orchestration only

## Next 5 Code Changes
1. Tune exhibition thresholds with live tests
- measure idle noise floor and set PRESENCE_THRESHOLD accordingly
- acceptance: stable 0 output when platform is unloaded

2. Verify tare robustness under repeated starts
- run at least 20 cold/warm startup cycles and log initial offsets
- acceptance: offset spread is within agreed range

3. Separate signal shaping from transport in src/main.cpp
- keep balance computation in one helper function
- keep OSC send in a separate helper function
- status: done
- acceptance: loop() is short and readable with clear responsibilities [done]

6. Centralize runtime constants and finalize main.cpp orchestration split [done]
- move pin, threshold, and startup constants to src/app_config.h [done]
- keep OSC send path in src/transport_osc.cpp [done]
- acceptance: src/main.cpp contains only setup and loop orchestration [done]

4. Define RS485 frame schema for migration
- finalize payload fields and CRC rules
- acceptance: schema ready for sender/receiver implementation

5. Prepare SLIP OSC output migration
- define packet format and framing integration point
- acceptance: replacement plan from current OSC path is documented
