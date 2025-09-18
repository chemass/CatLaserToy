#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\web\\README.md"
# Web Interface Organization

## Files Structure

### HTML Source File
- **`web/index.html`** - The main source HTML file for the laser toy control interface
  - Contains all CSS styling and JavaScript functionality
  - This is the file to edit when making changes to the web interface
  - Can be opened in a browser for testing (though WebSocket functionality requires the ESP32)

### Generated Header File
- **`WebInterface.h`** - Arduino header file containing the HTML as a C string constant
  - Generated from `web/index.html`
  - Included in the main Arduino sketch
  - Contains the `index_html[]` PROGMEM string used by the ESP32 web server

## Development Workflow

### Making Changes to the Web Interface

1. **Edit the source file**: Modify `web/index.html` with your changes
2. **Update the header file**: Copy the HTML content into `WebInterface.h`
   - Replace the content between `R"rawliteral(` and `)rawliteral";`
   - Keep the PROGMEM declaration and variable name intact
3. **Build and upload**: The Arduino IDE will now use the updated interface

### Automated Build Process (Future Enhancement)

For a more automated workflow, consider creating a build script that:
- Reads `web/index.html`
- Generates `WebInterface.h` automatically
- Integrates with the Arduino build process

## Benefits of This Organization

1. **Separation of Concerns**: HTML/CSS/JS separated from Arduino C++ code
2. **Better Development Experience**: Syntax highlighting and validation for web files
3. **Version Control**: Easier to track changes to web interface separately
4. **Testing**: Can open HTML file directly in browser for UI testing
5. **Collaboration**: Web developers can work on interface without Arduino knowledge

## Web Interface Features

The laser toy control interface includes:
- Interactive control box with drag/touch support
- Setup mode for boundary calibration with visual corner indicators
- Laser toggle and pattern buttons
- Real-time WebSocket communication with ESP32
- Throttling controls to prevent overwhelming the microcontroller
- Fine adjustment controls for precise laser positioning
- Responsive design for mobile and desktop use