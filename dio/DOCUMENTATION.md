# DIO Firmware Documentation (Arduino/Ethernet + Relay + Vehicle/Event Classification)

## 1) Files in this project
- **`dio/config.h`**: Hardware pin mapping + network/serial config constants.
- **`dio/dio.ino`**: Main firmware logic: Ethernet server, command parsing, heartbeat, hard-reset logic, input sensing + event classification.

---

## 2) High-level behavior
This firmware runs an **Ethernet TCP server** and communicates using **CSV-like string messages**.

It performs 4 main tasks:
1. **Receive commands** from a TCP client and update:
   - relay GPIO states
   - EEPROM settings (server IP/port/time)
   - RTC time
2. **Send periodic heartbeat (“Alive”)** messages reporting DI states.
3. **Detect prolonged hard-reset condition** (via `HARDRST` pin) and reset the MCU.
4. **Detect a vehicle/event** using DI input patterns, estimate:
   - direction (Forward/Reverse)
   - approximate speed & distance (based on trigger timing)
   - a vehicle class label (CAR/LCV/BUS/TRUCK/MAV/III/etc.) using distance/axle/height rules
   Then it sends an **event message**.

---

## 3) Configuration: `dio/config.h`
### 3.1 Ethernet & Serial
- `SERVER_PORT 1000`
- `SERVER_MAC = {DE, AD, BE, EF, FE, ED}`
- Default IP: `192.168.0.1` (used if EEPROM IP bytes invalid)
- `SerialInterface` is `Serial` at `115200`

### 3.2 Relay outputs
Defines 6 relay pins:
- `RELAY1 PC6`
- `RELAY2 PA9`
- `RELAY3 PA8`
- `RELAY4 PB15`
- `RELAY5 PB14`
- `RELAY6 PB13`

### 3.3 Digital inputs (sensors/triggers)
- `DI1 PD0`
- `DI2 PC_9`
- `DI3 PD1`
- `DI4 PD2`
- `DI5 PD3`
- `DI6 PD4`
- `DI7 PD_5`
- `DI8 PD_6`
- `DI9 PB3`
- `DI10 PB4`

### 3.4 Hard reset pin
- `HARDRST PC_3`

---

## 4) Main firmware: `dio/dio.ino`

### 4.1 Runtime composition (layers)
Think of the sketch in 5 layers:
1. **Hardware layer**
   - GPIO `pinMode`, `digitalRead`, `digitalWrite`
   - EEPROM read/write
   - RTC reads
2. **Network/message layer**
   - TCP server initialization
   - building outgoing strings
   - reading incoming TCP messages up to newline `\n`
3. **Command/application layer**
   - `parseCommand()` interprets CSV commands and updates EEPROM/RTC/relays
4. **Reliability layer**
   - `checkHardReset()` uses the `HARDRST` pin to trigger EEPROM changes + MCU reset
5. **Event-sensing & classification layer**
   - `avcc()` reads DI transitions, estimates timing/direction/speed/distance and classifies event

---

## 5) Function-by-function brief

### 5.1 `setup()`
Initializes everything once at boot:
1. Starts serial (`SerialInterface.begin(115200)`)
2. Sets relay pins as outputs
3. Sets DI pins as `INPUT_PULLDOWN`
4. Sets `HARDRST` as `INPUT_PULLUP`
5. Initializes Ethernet (SPI configuration + MAC/IP selection)
   - If EEPROM IP bytes `[0..3]` are valid (not `255`), uses them
   - Otherwise uses `ip_array` default (`192.168.0.1`)
6. Reads embedded sequence number:
   - If `EEPROM.read(7) != 0`, then `EmbeddedSeqN = EEPROMReadlong(25)`
7. Starts server: `server.begin()`
8. Sets all relays to `HIGH`

---

### 5.2 `loop()` (top-level orchestration)
Executed continuously:
```text
loop()
 ├─ str = getMessageEthernet()
 ├─ parseCommand(str)
 ├─ printAlive()
 ├─ checkHardReset()
 └─ avcc()
```

---

### 5.3 `getMessageEthernet()`
Reads one incoming TCP message from the current server-available client:
- `client = server.available()`
- waits briefly until connected and data becomes available
- reads bytes until newline `\n`
- returns the accumulated `String`

---

### 5.4 `parseCommand(String str)`
Interprets incoming CSV command strings.

Main command categories:
1. **File/HTTP-like request**: if message contains `txt`
   - Sends HTTP-like response headers:
     - `HTTP/1.1 200 OK`
     - `Content-Type: text/plain`
     - `Content-disposition:attachment; filename=<firstValue1>`
   - Stops client and restarts server listening

2. **CSV command** (when `str != ""`):
   - Splits `firstValue` as text before first comma
   - Routes based on `firstValue`:
     - `SET,....`  → writes EEPROM bytes `[0..3]`, then `NVIC_SystemReset()`
     - `SETS,<val>` → writes EEPROM[5], then reset
     - `SETP,<val>` → writes EEPROM[4], then reset
     - `DATE,a,b,c,d,e,f,g` → adjusts RTC with `rtc.adjust(DateTime(...))`
     - `R1..R6,<0/1>` → updates relay state via `updateRelayState(RELAYx, value)`

Returns `-1` at end (not used as a control signal).

---

## 6) Event-sensing & classification guide (`avcc()`)

### 6.1 Inputs used (meaning in code)
- **DI1**: start/end gate for an event (controls `loopAc` and `loopact`).
- **DI2**: transition-based counting for `hit` and timing capture for `t1`/`time1`.
- **DI6**: transition-based counting for `hit2`.
- **DI5**: transition-based counting for `hit3`.
- **DI4..DI7**: set `h1..h4` and compute `height = h1 + h2 + h3 + h4`.
- **DI1 & DI3** logic also updates axle counters:
  - `axil1` increments when `DI1` is high while `Prev_axil1==0`
  - `axil2` increments when `DI3` is high while `Prev_axil2==0`

> Note: The event classification primarily uses `hit` (and `hit2/hit3`) and `height`, plus computed `distanceU`.

### 6.2 State machine inside `avcc()`
Conceptually:
- When **DI1 goes HIGH**, event mode begins (`loopAc=1`, `loopact=1`).
- While in event mode, it watches other DI transitions to count hits and capture timing.
- When **DI1 goes LOW**, it ends event mode and computes velocity/distance and builds output message.

### 6.3 Direction calculation (F/R)
At event end:
- Uses relative ordering of captured times `t1` and `t2`.
- If `t1 < t2` → `forward = 1` → message uses **`F`**
- Else → `forward = 0` → message uses **`R`**

### 6.4 Speed & distance calculation
- Converts captured time differences from ms to seconds.
- Computes `velocity` (scaled by the constant `0.75` and then by `3.6`).
- Computes `distanceU` using velocity and the time window between triggers.

### 6.5 Vehicle type decision (high-level)
The code builds the event string `AVCNE` and appends a label based on:
- **DistanceU ranges** (several thresholds)
- **Axle/trigger estimate**: `((hit)/2)`
- **Height**: `height` (0..4)
- **Extra conditions**: `hit2` and `hit3`

The output label portion uses patterns like:
- `1,CAR,...`
- `2,LCV,...`
- `3,BUS,...` or `3,TRUCK,...`
- `5,MAV,...` / `6,MAV,...`
- fallback: `0,III,...`

### 6.6 After sending an event
When the message is printed (`server.println(AVCNE)` and `SerialInterface.println(AVCNE)`):
1. `EmbeddedSeqN++`
2. `EEPROMWritelong(25, EmbeddedSeqN)`
3. Reset event variables:
   - `AVCNE` back to `"START,AVCC,"`
   - `hit`, `hit2`, `hit3`, `axil1`, `height`, `t1`, `t2`, etc.
   - counters `h1..h4` to 0

---

## 7) Flow charts / diagrams

### 7.1 Full firmware flow chart (ASCII)
```text
                 ┌───────────────────────┐
                 │        loop()          │
                 └──────────┬────────────┘
                            │
                            v
                 ┌───────────────────────┐
                 │ getMessageEthernet()  │
                 └──────────┬────────────┘
                            │ str (may be "")
                            v
                 ┌───────────────────────┐
                 │    parseCommand(str)  │
                 └──────────┬────────────┘
                            │
                            v
                 ┌───────────────────────┐
                 │     printAlive()      │
                 └──────────┬────────────┘
                            │
                            v
                 ┌───────────────────────┐
                 │  checkHardReset()     │
                 └──────────┬────────────┘
                            │
                            v
                 ┌───────────────────────┐
                 │          avcc()        │
                 └───────────────────────┘
```

### 7.2 Message generation layer flow
```text
DI1 HIGH
  ├─ compute/store height (DI4..DI7)
  ├─ count hits (DI2 transitions) → hit
  ├─ count extra hits (DI6/DI5 transitions) → hit2/hit3
  └─ wait for DI1 LOW

DI1 LOW
  ├─ compute direction (t1 vs t2) → F/R
  ├─ compute velocity & distanceU
  ├─ classify vehicle label using thresholds
  └─ build CSV string → server.println(AVCNE)
```

---

## 8) Quick “how to read this code” guide
1. Start at **`loop()`** to understand call order.
2. For network behavior, read:
   - `getMessageEthernet()`
   - `parseCommand()`
3. For system reliability, read `checkHardReset()`.
4. For event classification, read `avcc()` carefully:
   - identify `loopAc` and event start/end conditions (DI1)
   - find counters (`hit`, `hit2`, `hit3`, `height`)
   - trace computed values (`t1`, `t2`, `velocity`, `distanceU`)
   - locate the large conditional block that appends vehicle label to `AVCNE`

---

## 9) Output strings (what the firmware sends)

### 9.1 Alive message
- Begins: `START,ALIVE,`
- Contains: DI1..DI10 states as `1/0`
- Appends terminator style: `,END` and `\0`

### 9.2 Event message (`AVCNE`)
- Begins: `START,AVCC,` and ends: `END`
- Contains:
  - network identifiers: `lanid`, `EmbeddedSeqN`
  - RTC timestamp: date and time
  - vehicle label segment: `X,<TYPE>,<stry>,<F/R>,...`
  - measured `distanceU` and speed/height mapping fields

---

## 10) Where to extend/modify
Common extension points:
- Add/adjust vehicle classification thresholds in the `avcc()` conditional block.
- Extend `parseCommand()` for new command types.
- Improve message robustness (limits, parsing, newline framing) in `getMessageEthernet()`.

