#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\.github\\copilot-instructions.md"

# Cat Laser Toy - AI Coding Assistant Guide

## Project Overview
This project is an ESP32-based automated laser toy for cats, featuring:
- 2-axis servo control
- WiFi connectivity and mDNS hostname
- Real-time WebSocket API for web/mobile control
- Persistent boundary storage (EEPROM)
- Manual and automated pattern modes
- Decoupled motion control architecture for testability

## Main Components & File Responsibilities

- **CatLaserToy.ino**: Main sketch; setup, loop, WiFi server, WebSocket event handler, high-level state management
- **Button.h/.cpp**: Debounced button input, long-press detection
- **Geometry.h/.cpp**: 2D point, normalized point, rectangle, quadrilateral, and geometric utilities (mapping, interpolation)
- **StorageManager.h/.cpp**: EEPROM persistence for 4-point boundary, setup flag, load/save logic
- **MotionPlanner.h/.cpp**: Geometric motion planning, pattern generation, boundary mapping, path interpolation
- **PatternGenerator.h/.cpp**: Abstract base and concrete classes for zigzag, spiral, random walk, figure-8, perimeter patterns
- **ServoController.h/.cpp**: Hardware abstraction for servo PWM, laser control, position tracking, limit enforcement
- **MotionExecutor.h/.cpp**: Command queue, non-blocking execution, immediate moves, emergency stop
- **WebInterface.h**: Embedded HTML for web UI (generated from `web/index.html`)
- **web/index.html**: Source for web interface (UI, JS, CSS)

## Motion Control Architecture

1. **Planning Layer** (`MotionPlanner`):
   - Generates motion sequences from geometric patterns
   - Maps normalized coordinates to boundary using bilinear interpolation
   - Hardware-agnostic, pure logic
2. **Hardware Layer** (`ServoController`):
   - Controls servos and laser via ESP32 LEDC PWM
   - Enforces position limits, tracks current position
   - Inverted laser logic (HIGH = OFF)
3. **Execution Layer** (`MotionExecutor`):
   - Queues and executes motion commands (up to 200)
   - Non-blocking execution in main loop
   - Immediate movement and emergency stop

## WebSocket API & Web Interface

- WebSocket endpoint: `/ws`
- Accepts JSON commands for manual and pattern control:
  - `{ "type": "move", "x": float, "y": float }` (normalized coordinates)
  - `{ "type": "zigzag-pattern" }`, `{ "type": "spiral-pattern" }`, etc.
  - `{ "type": "laser-toggle" }`, `{ "type": "enter-setup" }`, `{ "type": "store-point" }`
  - `{ "type": "move-up" }`, `{ "type": "move-down" }`, `{ "type": "move-left" }`, `{ "type": "move-right" }`
  - `{ "type": "stop-motion" }`, `{ "type": "stop-pattern" }`, `{ "type": "get-position" }`
- Status and error responses sent as JSON
- Web UI (in `web/index.html`) provides interactive controls, pattern buttons, setup mode, and visual feedback

## Boundary Storage & Setup Mode

- 4-point boundary (quadrilateral) stored in EEPROM
- Setup mode allows user to calibrate corners via web interface
- Validation flag (`0xDEADBEEF`) ensures boundary integrity
- All pattern generation and mapping use the stored boundary

## Pattern Generation

- Extendable via `PatternGenerator` subclasses (zigzag, spiral, random, figure-8, perimeter)
- Patterns generate normalized point sequences, mapped to boundary for servo movement
- Timing and speed controlled via `MotionCommand.duration_ms`

## Error Handling & Safety

- Motion queue overflow protection (max 200 commands)
- Laser auto-timeout (5 minutes)
- Emergency stop disables laser and clears queue
- All boundary loads check setup flag before use

## Build System & Artifacts

- Target: ESP32 (240MHz, 4MB flash)
- Key dependencies: ESPAsyncWebServer, AsyncTCP
- Build outputs: `.elf`, `.bin`, `.map` in `build/`
- Web interface: edit `web/index.html`, regenerate `WebInterface.h` for deployment

## Testing & Debugging

- Serial output at 115200 baud for status and debugging
- LED indicator (pin 2) blinks in setup mode
- Unit tests for geometry, pattern generation, and motion planning (see `tests/`)
- Integration tests with hardware mocks

## Development Workflow

1. Edit web UI in `web/index.html`, copy to `WebInterface.h` for deployment
2. Add new patterns by subclassing `PatternGenerator` and updating `MotionPlanner`
3. Use `motionExecutor.queueCommands()` for queued motion, `moveImmediate()` for direct moves
4. Always check `motionExecutor.isBusy()` before queuing new patterns
5. Use serial output and LED for debugging and status

## Common Tasks

- Add new pattern: subclass `PatternGenerator`, update `MotionPlanner`
- Change hardware: update `ServoController` pin assignments
- Debug motion: use serial output, test with manual moves and patterns
- Emergency stop: call `motionExecutor.stop()`
- Calibrate boundary: enter setup mode, store 4 points via web UI