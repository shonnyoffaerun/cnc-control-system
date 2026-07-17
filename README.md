# cnc-control-system

Control software for a bare wood CNC driven by a **MakerBase MKS DLC32 V2.1**
(ESP32). The project has two cooperating parts:

- **host/** — a Qt6 desktop application (CLI or GUI) that parses drawings,
  validates the cutting tool, visualizes the part, and streams commands to the
  board over a binary protocol.
- **firmware/** — grbl_esp32-based firmware that runs **on the MKS DLC32 itself**:
  it outputs step/dir signals, reads servo encoders, drives the spindle PWM and
  answers the binary protocol over UART2.

## How it maps to the requirements

| # | Requirement | Where |
|---|-------------|-------|
| 1 | Parser of drawings → machine commands | `host/cad_parser.{h,cpp}` (DXF, SVG, STEP, G-code) |
| 2 | Tool check: length, material, properties | `host/tool_manager.{h,cpp}`, `common/tool_schema.h` |
| 3 | Part visualization | `host/visualizer.{h,cpp}` (Qt `QPainter`) |
| 4 | Performance on the board | `firmware/` (runs on the ESP32 itself) |
| 5 | Interface | Qt6 GUI (`host/mainwindow.{h,cpp}`) + CLI fallback in `host/main.cpp` |
| 6 | Signal send / encoder read | `firmware/step_driver`, `firmware/encoder_reader`, `firmware/spindle_control`, `firmware/protocol_handler` |
| 7 | grbl_esp32 based | `firmware/src/main.cpp` boots `grbl_init()` and injects G-code via `grbl_stream_write()` |

## Wire protocol

Host and firmware share one header, `common/protocol.h`, so the framing is
identical on both ends: a `ProtocolFrame` (`magic | type | len | seq | payload | crc16`).
Textual grbl commands are also supported on UART0 (USB) for compatibility.

```
host (PC)  ──UART2 binary (115200)──▶  MKS DLC32  ──step/dir──▶  drivers/servos
             ◀── telemetry (pos from encoders)──                 ◀── encoder A/B ──
```

## Building the host (Qt6 + CMake)

```bash
cmake -S . -B build
cmake --build build -j
./build/cnc-control-system            # opens the Qt GUI
./build/cnc-control-system --port /dev/ttyACM0 --baud 115200
```

Edit `config.json` to set the serial port and `useGui` (set `false` for a
minimal CLI loop).

## Building the firmware (PlatformIO)

```bash
cd firmware
pio run -e mks_dlc32        # compile
pio run -e mks_dlc32 -t upload
pio device monitor          # grbl text console on USB (UART0)
```

The binary protocol talks to UART2 (GPIO21/22 @ 115200). Pin map and encoder
resolution are configured in `firmware/platformio.ini`.

## Project layout

```
cnc-control-system/
├── CMakeLists.txt            # host build (Qt6)
├── config.json               # default host config
├── src/
│   ├── common/               # shared, Qt-free headers
│   │   ├── protocol.h/.cpp   # binary wire framing (host + firmware)
│   │   ├── gcode_types.h     # G-code enums / status structs
│   │   └── tool_schema.h     # packed tool description + checks
│   └── host/                 # desktop app
│       ├── main.cpp/.h       # app bootstrap, CLI/Qt selection
│       ├── mainwindow.*      # Qt GUI
│       ├── serial_bridge.*   # UART + queue + response parsing
│       ├── cad_parser.*      # DXF/SVG/STEP/G-code → toolpath
│       ├── tool_manager.*    # tool DB, validation, G-code gen
│       └── visualizer.*      # toolpath + state rendering
└── firmware/                 # runs ON the MKS DLC32 (PlatformIO)
    ├── platformio.ini
    ├── include/              # encoder_reader.h, step_driver.h,
    │                         #   spindle_control.h, protocol_handler.h
    └── src/
        ├── main.cpp          # grbl_esp32 boot + glue
        ├── protocol_handler.cpp
        ├── encoder_reader.cpp
        ├── step_driver.cpp
        └── spindle_control.cpp
```
