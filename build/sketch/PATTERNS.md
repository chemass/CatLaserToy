#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\PATTERNS.md"
# Pattern Control Features

## New Web Interface Buttons

The Cat Laser Toy web interface now includes dedicated pattern buttons for easy control:

### Available Patterns

🔳 **Zigzag** - Classic back-and-forth pattern across the boundary  
🌀 **Spiral** - Spiral motion from center outward  
🎲 **Random Walk** - Unpredictable random movement  
∞ **Figure-8** - Smooth figure-8 loops  
⬜ **Perimeter** - Traces around the boundary edges  
⏹️ **Stop** - Immediately halt current pattern  

### User Interface Features

- **Visual Feedback**: Buttons disable while pattern is running
- **Status Display**: Shows current pattern state ("Ready to play", "Pattern started...", etc.)
- **One-Click Control**: Simply click any pattern button to start
- **Emergency Stop**: Red stop button to halt any running pattern
- **Automatic Reset**: Buttons re-enable when pattern completes

### Pattern Requirements

- **Boundary Setup Required**: All 4 corner points must be configured before patterns work
- **WebSocket Connection**: Real-time communication between web interface and ESP32
- **Queue Management**: Only one pattern can run at a time

### WebSocket Commands

The interface sends these JSON commands to the ESP32:

```json
{ "type": "zigzag-pattern" }
{ "type": "spiral-pattern" }
{ "type": "random-pattern" }
{ "type": "figure8-pattern" }
{ "type": "perimeter-pattern" }
{ "type": "stop-pattern" }
```

### ESP32 Responses

The ESP32 sends status updates back to the web interface:

- `"pattern-complete"` - Pattern finished successfully
- `"pattern-stopped"` - Pattern manually stopped
- Updates button states automatically

### Error Handling

- Patterns only start if boundary is properly configured
- Prevents starting new patterns while one is already running  
- WebSocket reconnection on connection loss
- Fallback to IP address if hostname fails

### Button Styling

- **Blue buttons**: Available pattern options
- **Red button**: Emergency stop (only enabled during patterns)
- **Disabled state**: Gray appearance when unavailable
- **Hover effects**: Visual feedback on interaction

This provides a much more user-friendly way to control the laser toy compared to the previous automatic pattern triggering!