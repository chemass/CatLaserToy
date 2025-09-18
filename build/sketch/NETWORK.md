#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\NETWORK.md"
# Cat Laser Toy - Network Access Guide

## Hostname Configuration

The Cat Laser Toy now supports mDNS (multicast DNS) for easy network access using a friendly hostname instead of remembering IP addresses.

### Default Hostname

**Default hostname:** `cattoy`  
**Access URL:** `http://cattoy.local`

### Accessing the Device

You can access the laser toy control interface using either method:

1. **Hostname (Recommended):** `http://cattoy.local`
   - Easy to remember
   - Works even if IP address changes
   - Supported on most modern devices

2. **IP Address:** `http://192.168.x.x` (shown on device startup and in web interface)
   - Traditional method
   - Always works as fallback

### Customizing the Hostname

To change the hostname, edit the `hostname` variable in `CatLaserToy.ino`:

```cpp
const char* hostname = "cattoy"; // Change this to your preferred name
```

**Examples:**
- `"lasertoy"` → Access via `http://lasertoy.local`
- `"catplay"` → Access via `http://catplay.local`
- `"bellatoy"` → Access via `http://bellatoy.local`

**Hostname Rules:**
- Use only letters, numbers, and hyphens
- Start and end with a letter or number
- Keep it short and memorable
- No spaces or special characters

### Device Compatibility

**✅ Supported Devices:**
- Windows 10/11 (with Bonjour/iTunes installed)
- macOS (built-in support)
- iOS devices (iPhone/iPad)
- Android devices (most modern versions)
- Linux (with avahi-daemon)

**⚠️ May Need Configuration:**
- Older Windows versions (install Bonjour)
- Some corporate networks (may block mDNS)

### Troubleshooting

**If hostname doesn't work:**
1. Try the IP address instead (shown in serial monitor and web interface)
2. Ensure your device supports mDNS
3. Check if your router/network allows mDNS traffic
4. Restart your device's network connection

**Serial Monitor Output:**
```
Connected to WiFi
IP address: 192.168.1.100
mDNS responder started. Access via: http://cattoy.local
```

### Technical Details

- **Protocol:** mDNS (Multicast DNS)
- **Port:** 80 (HTTP)
- **Service:** HTTP web server
- **Library:** ESP32 ESPmDNS

The device automatically advertises itself on the local network, making it discoverable by name rather than IP address.