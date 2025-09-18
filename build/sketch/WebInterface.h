#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\WebInterface.h"
#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

// HTML content as PROGMEM string
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Laser Toy Control</title>
    <style>
        /* Reset and base styles */
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            line-height: 1.6;
            color: #333;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 1rem;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 2rem;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
        }
        
        h1 {
            text-align: center;
            margin-bottom: 2rem;
            color: #2c3e50;
            font-size: 2rem;
            font-weight: 700;
        }
        
        /* Network info banner */
        .network-info {
            margin-bottom: 1.5rem;
            padding: 1rem;
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
            border-radius: 12px;
            font-size: 0.9rem;
            text-align: center;
        }
        
        .network-info a {
            color: white;
            text-decoration: underline;
        }
        
        /* Setup banner */
        #setup-banner {
            background: linear-gradient(135deg, #ffd89b 0%, #19547b 100%);
            color: white;
            padding: 1rem;
            margin-bottom: 1.5rem;
            border-radius: 12px;
            font-weight: 600;
            text-align: center;
        }
        
        /* Control buttons */
        .control-buttons {
            display: flex;
            flex-wrap: wrap;
            gap: 1rem;
            margin-bottom: 2rem;
            justify-content: center;
        }
        
        .btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 0.8rem 1.5rem;
            border-radius: 50px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
            position: relative;
            overflow: hidden;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.3);
        }
        
        .btn:active {
            transform: translateY(0);
        }
        
        .btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
            transition: left 0.5s;
        }
        
        .btn:hover::before {
            left: 100%;
        }
        
        /* Control box - mobile first */
        .control-container {
            position: relative;
            display: flex;
            justify-content: center;
            margin-bottom: 2rem;
        }
        
        #control-box {
            width: 100%;
            max-width: 400px;
            aspect-ratio: 1;
            border: 3px solid #667eea;
            background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
            position: relative;
            touch-action: none;
            border-radius: 20px;
            box-shadow: inset 0 4px 8px rgba(0, 0, 0, 0.1);
        }
        
        #control-box::before {
            content: '';
            position: absolute;
            top: 2em;
            left: 2em;
            right: 2em;
            bottom: 2em;
            border: 2px dashed rgba(102, 126, 234, 0.5);
            pointer-events: none;
            border-radius: 12px;
        }
        
        #pointer {
            position: absolute;
            width: 20px;
            height: 20px;
            background: radial-gradient(circle, #e74c3c, #c0392b);
            border-radius: 50%;
            pointer-events: none;
            left: 0;
            top: 0;
            transform: translate(-10px, -10px);
            display: none;
            box-shadow: 0 4px 12px rgba(231, 76, 60, 0.4);
            border: 2px solid white;
        }
        
        /* Fine controls and setup elements */
        .control-container {
            position: relative;
            display: flex;
            justify-content: center;
        }
        
        .fine-control {
            position: absolute;
            background: rgba(102, 126, 234, 0.3);
            cursor: pointer;
            z-index: 10;
            display: none;
            transition: background 0.3s ease;
        }
        
        .fine-control:hover {
            background: rgba(102, 126, 234, 0.6);
        }
        
        .fine-control.top, .fine-control.bottom {
            height: 20px;
            left: 20px;
            right: 20px;
            border-radius: 10px;
        }
        
        .fine-control.left, .fine-control.right {
            width: 20px;
            top: 20px;
            bottom: 20px;
            border-radius: 10px;
        }
        
        .fine-control.top { top: 0; }
        .fine-control.bottom { bottom: 0; }
        .fine-control.left { left: 0; }
        .fine-control.right { right: 0; }
        
        .corner-indicator {
            position: absolute;
            width: 50px;
            height: 50px;
            background: linear-gradient(135deg, #e74c3c, #c0392b);
            border: 3px solid white;
            border-radius: 50%;
            color: white;
            font-weight: bold;
            display: none;
            align-items: center;
            justify-content: center;
            font-size: 14px;
            z-index: 15;
            pointer-events: none;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
        }
        
        .corner-indicator.active {
            background: linear-gradient(135deg, #27ae60, #2ecc71);
            animation: pulse 1.5s infinite;
            display: flex;
        }
        
        @keyframes pulse {
            0%, 100% { 
                opacity: 0.8; 
                transform: scale(1);
            }
            50% { 
                opacity: 1; 
                transform: scale(1.1);
            }
        }
        
        .corner-indicator.tl { top: -25px; left: -25px; }
        .corner-indicator.tr { top: -25px; right: -25px; }
        .corner-indicator.br { bottom: -25px; right: -25px; }
        .corner-indicator.bl { bottom: -25px; left: -25px; }
        
        /* Pattern controls */
        .pattern-controls {
            margin: 2rem 0;
            padding: 2rem;
            background: linear-gradient(135deg, #f8f9fa 0%, #e9ecef 100%);
            border-radius: 20px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
        }
        
        .pattern-controls h3 {
            margin-bottom: 1.5rem;
            color: #2c3e50;
            font-size: 1.5rem;
            text-align: center;
        }
        
        .pattern-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 1rem;
            margin-bottom: 1.5rem;
        }
        
        .pattern-btn {
            background: linear-gradient(135deg, #3498db, #2980b9);
            color: white;
            border: none;
            padding: 1rem;
            border-radius: 15px;
            font-size: 0.9rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(52, 152, 219, 0.3);
        }
        
        .pattern-btn:hover:not(:disabled) {
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(52, 152, 219, 0.4);
        }
        
        .pattern-btn:disabled {
            background: linear-gradient(135deg, #bdc3c7, #95a5a6);
            cursor: not-allowed;
            transform: none;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
        }
        
        .stop-btn {
            background: linear-gradient(135deg, #e74c3c, #c0392b) !important;
            box-shadow: 0 4px 15px rgba(231, 76, 60, 0.3) !important;
        }
        
        .stop-btn:hover:not(:disabled) {
            box-shadow: 0 8px 25px rgba(231, 76, 60, 0.4) !important;
        }
        
        .pattern-status {
            text-align: center;
            padding: 1rem;
            background: rgba(255, 255, 255, 0.7);
            border-radius: 12px;
            font-weight: 600;
            color: #2c3e50;
            border: 2px solid rgba(102, 126, 234, 0.2);
        }
        
        /* Responsive design for larger screens */
        @media (min-width: 768px) {
            body {
                padding: 2rem;
            }
            
            .container {
                padding: 3rem;
            }
            
            h1 {
                font-size: 2.5rem;
            }
            
            #control-box {
                max-width: 500px;
            }
            
            .pattern-buttons {
                grid-template-columns: repeat(3, 1fr);
            }
        }
        
        @media (min-width: 1024px) {
            #control-box {
                max-width: 600px;
            }
            
            .pattern-buttons {
                grid-template-columns: repeat(6, 1fr);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Cat Laser Toy Interface</h1>
        
        <div class="network-info">
            <strong>Access this device:</strong> <a href="http://cattoy.local">http://cattoy.local</a> or <span id="ip-address">Loading IP...</span>
        </div>
        
        <div id="setup-banner" style="display:none;">
            <span id="setup-text">Setup Mode: Position the laser at the TOP-LEFT corner, then click "Store Point".</span>
        </div>
        
        <div class="control-buttons">
            <button id="laser-btn" class="btn">Toggle Laser</button>
            <button id="setup-btn" class="btn">Enter Setup Mode</button>
            <button id="store-btn" class="btn" style="display:none;">Store TOP-LEFT Point</button>
        </div>
        
        <div class="pattern-controls">
            <h3>Laser Patterns</h3>
            <div class="pattern-buttons">
                <button id="zigzag-btn" class="pattern-btn">Zigzag</button>
                <button id="spiral-btn" class="pattern-btn">Spiral</button>
                <button id="random-btn" class="pattern-btn">Random Walk</button>
                <button id="figure8-btn" class="pattern-btn">Figure-8</button>
                <button id="perimeter-btn" class="pattern-btn">Perimeter</button>
                <button id="stop-btn" class="pattern-btn stop-btn">Stop</button>
            </div>
            <div class="pattern-status">
                <span id="pattern-status">Ready to play</span>
            </div>
        </div>
    
    <div class="control-container">
        <div id="control-box">
            <div id="pointer"></div>
            <!-- Corner indicators -->
            <div class="corner-indicator tl active" id="corner-tl">TL</div>
            <div class="corner-indicator tr" id="corner-tr">TR</div>
            <div class="corner-indicator br" id="corner-br">BR</div>
            <div class="corner-indicator bl" id="corner-bl">BL</div>
        </div>
        <!-- Fine control borders -->
        <div class="fine-control top" data-axis="y" data-direction="-1" title="Move Up"></div>
        <div class="fine-control bottom" data-axis="y" data-direction="1" title="Move Down"></div>
        <div class="fine-control left" data-axis="x" data-direction="-1" title="Move Left"></div>
        <div class="fine-control right" data-axis="x" data-direction="1" title="Move Right"></div>
    </div>
    <script>
        let inSetupMode = false;
        let pointsStored = 0;
        
        // Display current IP address
        document.getElementById('ip-address').textContent = `http://${location.hostname}:${location.port || 80}`;
        
        const socket = new WebSocket(`ws://${location.host}/ws`);
        socket.onopen = () => {
            console.log("WebSocket connected");
            // Request initial position
            socket.send(JSON.stringify({ type: 'get-position' }));
        };
        socket.onmessage = (event) => {
            console.log("Received:", event.data);
            
            // Handle JSON messages
            try {
                const data = JSON.parse(event.data);
                if (data.type === 'position') {
                    // Set initial pointer position
                    setPointer(data.x, data.y);
                    lastPointerX = data.x;
                    lastPointerY = data.y;
                    return;
                }
            } catch (e) {
                // Handle non-JSON messages (legacy format)
            }
            
            // Listen for setup mode signals (legacy format)
            if (event.data === 'setup') {
                inSetupMode = true;
                pointsStored = 0; // Reset points counter
                updateSetupUI();
                showSetupElements();
                document.getElementById('setup-banner').style.display = '';
                document.getElementById('store-btn').style.display = '';
                document.getElementById('setup-btn').style.display = 'none';
                document.getElementById('store-btn').disabled = false;
            } else if (event.data === 'setup-done') {
                inSetupMode = false;
                document.getElementById('setup-banner').style.display = 'none';
                document.getElementById('store-btn').style.display = 'none';
                document.getElementById('setup-btn').style.display = '';
                hideSetupElements();
            } else if (event.data === 'pattern-complete') {
                updatePatternStatus('Pattern completed');
                setPatternButtonsState(true);
            } else if (event.data === 'pattern-stopped') {
                updatePatternStatus('Pattern stopped');
                setPatternButtonsState(true);
            }
        };
        socket.onclose = (event) => {
            console.log("WebSocket disconnected:", event);
            setTimeout(() => {
                console.log("Attempting to reconnect...");
                location.reload(); // Simple reconnection by reloading page
            }, 2000);
        };
        socket.onerror = (error) => {
            console.error("WebSocket error:", error);
        };

        // Laser toggle button
        const laserBtn = document.getElementById('laser-btn');
        laserBtn.onclick = () => {
            socket.send(JSON.stringify({ type: 'laser-toggle' }));
        };

        // Setup mode button
        const setupBtn = document.getElementById('setup-btn');
        setupBtn.onclick = () => {
            socket.send(JSON.stringify({ type: 'enter-setup' }));
        };

        // Store point button
        const storeBtn = document.getElementById('store-btn');
        storeBtn.onclick = () => {
            socket.send(JSON.stringify({ type: 'store-point' }));
            pointsStored++;
            updateSetupUI();
            if (pointsStored >= 4) {
                storeBtn.disabled = true;
            }
        };

        // Pattern buttons
        const patternButtons = {
            'zigzag-btn': 'zigzag-pattern',
            'spiral-btn': 'spiral-pattern', 
            'random-btn': 'random-pattern',
            'figure8-btn': 'figure8-pattern',
            'perimeter-btn': 'perimeter-pattern'
        };

        // Add click handlers for pattern buttons
        Object.entries(patternButtons).forEach(([buttonId, command]) => {
            const button = document.getElementById(buttonId);
            button.onclick = () => {
                socket.send(JSON.stringify({ type: command }));
                updatePatternStatus(`${button.textContent} pattern started...`);
                setPatternButtonsState(false);
            };
        });

        // Stop button
        const stopBtn = document.getElementById('stop-btn');
        stopBtn.onclick = () => {
            socket.send(JSON.stringify({ type: 'stop-pattern' }));
            updatePatternStatus('Pattern stopped');
            setPatternButtonsState(true);
        };

        // Helper functions for pattern UI
        function updatePatternStatus(message) {
            document.getElementById('pattern-status').textContent = message;
        }

        function setPatternButtonsState(enabled) {
            Object.keys(patternButtons).forEach(buttonId => {
                document.getElementById(buttonId).disabled = !enabled;
            });
            document.getElementById('stop-btn').disabled = enabled;
        }

        // Corner order: Top-Left, Top-Right, Bottom-Right, Bottom-Left
        const cornerOrder = ['TL', 'TR', 'BR', 'BL'];
        const cornerNames = ['TOP-LEFT', 'TOP-RIGHT', 'BOTTOM-RIGHT', 'BOTTOM-LEFT'];
        const cornerInstructions = [
            'Position the laser at the TOP-LEFT corner, then click "Store Point".',
            'Position the laser at the TOP-RIGHT corner, then click "Store Point".',
            'Position the laser at the BOTTOM-RIGHT corner, then click "Store Point".',
            'Position the laser at the BOTTOM-LEFT corner, then click "Store Point".'
        ];

        function updateSetupUI() {
            if (!inSetupMode) return;
            
            const currentCorner = cornerOrder[pointsStored];
            const currentName = cornerNames[pointsStored];
            const currentInstruction = cornerInstructions[pointsStored];
            
            // Update UI text
            document.getElementById('setup-text').textContent = 'Setup Mode: ' + currentInstruction;
            document.getElementById('store-btn').textContent = 'Store ' + currentName + ' Point';
            
            // Update corner indicators
            document.querySelectorAll('.corner-indicator').forEach(el => {
                el.classList.remove('active');
                el.style.display = 'none';
            });
            if (pointsStored < 4) {
                const activeCorner = document.getElementById('corner-' + currentCorner.toLowerCase());
                activeCorner.classList.add('active');
                activeCorner.style.display = 'flex';
            }
        }

        function showSetupElements() {
            // Show fine control borders
            document.querySelectorAll('.fine-control').forEach(el => {
                el.style.display = 'block';
            });
        }

        function hideSetupElements() {
            // Hide corner indicators
            document.querySelectorAll('.corner-indicator').forEach(el => {
                el.classList.remove('active');
                el.style.display = 'none';
            });
            // Hide fine control borders
            document.querySelectorAll('.fine-control').forEach(el => {
                el.style.display = 'none';
            });
        }

        function resetCornerIndicators() {
            hideSetupElements();
        }

        // Control box logic
        const box = document.getElementById('control-box');
        const pointer = document.getElementById('pointer');
        let isDragging = false;
        let lastSendTime = 0;
        let sendThrottleMs = 40; // Set to 40ms for responsive control

        function sendPointer(x, y) {
            // Throttle sending to prevent flooding the ESP32
            const now = Date.now();
            if (now - lastSendTime < sendThrottleMs) {
                return; // Skip this update
            }
            lastSendTime = now;
            
            // x, y are in [0, 1] range
            if (inSetupMode) {
                // In setup mode, send direct servo positioning command
                socket.send(JSON.stringify({ type: 'setup-move', x, y }));
            } else {
                // Normal mode, use boundary mapping
                socket.send(JSON.stringify({ type: 'move', x, y }));
            }
        }

        function setPointer(x, y) {
            pointer.style.left = (x * box.clientWidth) + 'px';
            pointer.style.top = (y * box.clientHeight) + 'px';
            pointer.style.display = 'block';
        }

        function handleBoxEvent(evt) {
            let rect = box.getBoundingClientRect();
            let clientX, clientY;
            if (evt.touches) {
                clientX = evt.touches[0].clientX;
                clientY = evt.touches[0].clientY;
            } else {
                clientX = evt.clientX;
                clientY = evt.clientY;
            }
            
            // Calculate buffer zone (2em in pixels)
            const computedStyle = getComputedStyle(box);
            const fontSize = parseFloat(computedStyle.fontSize);
            const bufferPx = 2 * fontSize; // 2em in pixels
            
            // Adjust coordinates to account for buffer zone
            let x = (clientX - rect.left - bufferPx) / (rect.width - 2 * bufferPx);
            let y = (clientY - rect.top - bufferPx) / (rect.height - 2 * bufferPx);
            
            // Clamp to [0, 1] range
            x = Math.max(0, Math.min(1, x));
            y = Math.max(0, Math.min(1, y));
            
            // Map back to full box coordinates for visual positioning
            const visualX = x * (rect.width - 2 * bufferPx) / rect.width + bufferPx / rect.width;
            const visualY = y * (rect.height - 2 * bufferPx) / rect.height + bufferPx / rect.height;
            
            setPointer(visualX, visualY);
            sendPointer(x, y);
        }

        box.addEventListener('mousedown', e => { isDragging = true; handleBoxEvent(e); });
        box.addEventListener('mousemove', e => { if (isDragging) handleBoxEvent(e); });
        box.addEventListener('mouseup', e => { isDragging = false; });
        box.addEventListener('mouseleave', e => { isDragging = false; });

        box.addEventListener('touchstart', e => { isDragging = true; handleBoxEvent(e); });
        box.addEventListener('touchmove', e => { if (isDragging) handleBoxEvent(e); });
        box.addEventListener('touchend', e => { isDragging = false; });
        box.addEventListener('touchcancel', e => { isDragging = false; });

        // Fine control borders
        document.querySelectorAll('.fine-control').forEach(control => {
            control.addEventListener('click', (e) => {
                const axis = e.target.dataset.axis;
                const direction = parseInt(e.target.dataset.direction);
                const step = 0.02; // 2% step size
                
                let newX = lastPointerX || 0.5;
                let newY = lastPointerY || 0.5;
                
                if (axis === 'x') {
                    newX += direction * step;
                    newX = Math.max(0, Math.min(1, newX));
                } else if (axis === 'y') {
                    newY += direction * step;
                    newY = Math.max(0, Math.min(1, newY));
                }
                
                // Update visual position
                const computedStyle = getComputedStyle(box);
                const fontSize = parseFloat(computedStyle.fontSize);
                const bufferPx = 2 * fontSize;
                const rect = box.getBoundingClientRect();
                const visualX = newX * (rect.width - 2 * bufferPx) / rect.width + bufferPx / rect.width;
                const visualY = newY * (rect.height - 2 * bufferPx) / rect.height + bufferPx / rect.height;
                
                setPointer(visualX, visualY);
                sendPointer(newX, newY);
                
                lastPointerX = newX;
                lastPointerY = newY;
            });
        });

        // Initialize UI
        updateSetupUI();
        setPatternButtonsState(true); // Enable pattern buttons initially
    </script>
    </div>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H