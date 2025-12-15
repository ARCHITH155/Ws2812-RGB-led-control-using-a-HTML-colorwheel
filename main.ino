#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

// ---------------------------------------------------
// 1. WIFI SETTINGS
// ---------------------------------------------------
const char* ssid     = "your wifi name;
const char* password = "wifi password";

// ---------------------------------------------------
// 2. HARDWARE SETTINGS
// ---------------------------------------------------
#define LED_PIN     5
#define LED_COUNT   40
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define MAX_SAFE_BRIGHTNESS 255 

CRGB leds[LED_COUNT];
WebServer server(80);

// -- State Variables --
// 0=Solid, 1=Rainbow, 2=Raindrop
int currentMode = 0; 
uint8_t gHue = 0; 

// ---------------------------------------------------
// 3. HTML CODE (Simplified & Robust JS)
// ---------------------------------------------------
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Light Controller</title>
  <style>
    body { font-family: sans-serif; background: #eee; padding: 20px; text-align: center; }
    .card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); max-width: 400px; margin: 0 auto 20px auto; }
    
    h2 { margin-top: 0; color: #333; }
    
    /* Range Slider */
    input[type=range] { width: 100%; margin: 10px 0; }
    
    /* Buttons */
    .btn-group { display: flex; gap: 10px; justify-content: center; margin-bottom: 15px; }
    button { padding: 10px 15px; border: none; border-radius: 8px; background: #ddd; cursor: pointer; font-weight: bold; }
    button.active { background: #007bff; color: white; }

    /* Color Input */
    input[type=color] { border: none; width: 100px; height: 100px; cursor: pointer; }
    
    .status { font-size: 0.8em; color: #666; margin-top: 10px; }
  </style>
</head>
<body>

  <div class="card">
    <h2>Brightness</h2>
    <input type="range" id="bri" min="0" max="255" value="150" onchange="sendBri(this.value)" oninput="sendBri(this.value)">
    <div id="briVal">150</div>
  </div>

  <div class="card">
    <h2>Effects</h2>
    <div class="btn-group">
        <button id="btn-solid" class="active" onclick="setMode(0)">Solid</button>
        <button id="btn-rainbow" onclick="setMode(1)">Rainbow</button>
        <button id="btn-rain" onclick="setMode(2)">Rain</button>
    </div>
  </div>

  <div class="card" id="colorCard">
    <h2>Color Control</h2>
    <div class="btn-group">
        <button onclick="setSeg(1)">Zone 1</button>
        <button onclick="setSeg(2)">Zone 2</button>
        <button onclick="setSeg(3)">Zone 3</button>
        <button onclick="setSeg(4)">Zone 4</button>
        <button onclick="setSeg(0)">ALL</button>
    </div>
    <input type="color" id="colorPicker" value="#ff0000" oninput="sendColor(this.value)">
    <div class="status" id="status">Ready</div>
  </div>

<script>
  let currentSeg = 1;
  let lastSent = 0;

  // --- BRIGHTNESS ---
  function sendBri(val) {
    document.getElementById('briVal').innerText = val;
    // Simple debounce: only send if 50ms passed to prevent crashing ESP
    const now = Date.now();
    if (now - lastSent > 50) {
        fetch('setBri?val=' + val);
        lastSent = now;
    }
  }

  // --- MODES ---
  function setMode(m) {
    // Update Buttons UI
    document.querySelectorAll('button').forEach(b => b.classList.remove('active'));
    if(m==0) document.getElementById('btn-solid').classList.add('active');
    if(m==1) document.getElementById('btn-rainbow').classList.add('active');
    if(m==2) document.getElementById('btn-rain').classList.add('active');
    
    // Toggle Color Card visibility
    const cCard = document.getElementById('colorCard');
    if (m === 0) cCard.style.opacity = '1'; 
    else cCard.style.opacity = '0.4';

    fetch('setMode?m=' + m);
  }

  // --- SEGMENTS ---
  function setSeg(s) {
    currentSeg = s;
    document.getElementById('status').innerText = "Selected Zone: " + (s==0 ? "ALL" : s);
  }

  // --- COLOR ---
  function sendColor(hex) {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);
    
    const now = Date.now();
    if (now - lastSent > 50) {
        document.getElementById('status').innerText = "Sending...";
        fetch(`setCol?s=${currentSeg}&r=${r}&g=${g}&b=${b}`)
            .then(() => document.getElementById('status').innerText = "Done");
        lastSent = now;
    }
  }
</script>
</body>
</html>
)rawliteral";

// ---------------------------------------------------
// 4. ANIMATION LOGIC
// ---------------------------------------------------
void runRainbow() {
  fill_rainbow(leds, LED_COUNT, gHue, 7);
}

void runRaindrop() {
  fadeToBlackBy(leds, LED_COUNT, 20);
  if (random8() < 30) {
    leds[random16(LED_COUNT)] = CRGB::White;
  }
}

// ---------------------------------------------------
// 5. SERVER HANDLERS (With Debug Prints)
// ---------------------------------------------------
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleBri() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    Serial.print("Received Brightness: "); Serial.println(val); // DEBUG
    
    FastLED.setBrightness(val);
    FastLED.show(); // Force update immediately
    
    server.send(200, "text/plain", "OK");
  }
}

void handleMode() {
  if (server.hasArg("m")) {
    int m = server.arg("m").toInt();
    Serial.print("Mode Changed to: "); Serial.println(m); // DEBUG
    
    currentMode = m;
    if (currentMode == 0) {
      // If switching back to solid, don't clear, just wait for color input
    } else {
      FastLED.clear(); // Clear strip for animation start
    }
    server.send(200, "text/plain", "OK");
  }
}

void handleColor() {
  if (server.hasArg("r")) {
    int s = server.arg("s").toInt();
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();

    // Debug Print
    Serial.printf("Color: Zone %d -> R%d G%d B%d\n", s, r, g, b);

    // Only apply if we are effectively in solid mode (or override)
    // We force mode to 0 (Solid) if user picks a color
    if (currentMode != 0) currentMode = 0; 

    int start = 0;
    int end = LED_COUNT;

    if (s == 1) { start=0; end=10; }
    if (s == 2) { start=10; end=20; }
    if (s == 3) { start=20; end=30; }
    if (s == 4) { start=30; end=40; }

    for(int i=start; i<end; i++) {
      leds[i] = CRGB(r,g,b);
    }
    FastLED.show();
    server.send(200, "text/plain", "OK");
  }
}

// ---------------------------------------------------
// 6. SETUP & LOOP
// ---------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(150);
  FastLED.clear();
  FastLED.show();

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n--- CONNECTED ---");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/setBri", handleBri);
  server.on("/setMode", handleMode);
  server.on("/setCol", handleColor);
  
  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  server.handleClient();

  // Animation Loop (Runs every 30ms)
  EVERY_N_MILLISECONDS(30) {
    if (currentMode == 1) { // Rainbow
      gHue++;
      runRainbow();
      FastLED.show();
    }
    else if (currentMode == 2) { // Rain
      runRaindrop();
      FastLED.show();
    }
    // Mode 0 (Solid) does nothing here, it waits for web input
  }
}
