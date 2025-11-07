#include <SoftwareSerial.h>
SoftwareSerial lora(10, 11); // RX, TX

// LED pin assignments
const int LED_R = 6;  // Red   -> 'R'
const int LED_Y = 5;  // Yellow-> 'Y'
const int LED_G = 4;  // Green -> 'G'
const int LED_B = 3;  // Blue  -> 'B'
const int LED_W = 2;  // White -> 'W'

void setup() {
  Serial.begin(115200);
  lora.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_W, OUTPUT);
  allOff();

  Serial.println("=== LoRa Receiver Test (LED Highlighter) ===");
  delay(1000);

  // LoRa module setup (unchanged)
  sendCommand("AT");
  sendCommand("AT+ADDRESS=2");
  sendCommand("AT+NETWORKID=5");
  sendCommand("AT+BAND=915000000");

  Serial.println("[INFO] Receiver ready and waiting for messages...");
}

void loop() {
  readLoRaResponse();
}

// --- Utility: send AT command and print reply ---
void sendCommand(String cmd) {
  Serial.print(">> "); Serial.println(cmd);
  lora.println(cmd);
  delay(300);
  while (lora.available()) {
    Serial.write(lora.read());
  }
  Serial.println();
}

// --- Read LoRa messages and update LEDs ---
void readLoRaResponse() {
  // Read line-by-line; modules usually end with \r\n
  while (lora.available()) {
    String line = lora.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    Serial.print("[RECEIVED] ");
    Serial.println(line);

    String payload = extractPayload(line);   // <-- only DATA field
    if (payload.length() == 0) {
      Serial.println("[PARSE] No payload found; LEDs unchanged.");
      continue;
    }

    Serial.print("[PAYLOAD] ");
    Serial.println(payload);
    handleMessage(payload);
  }
}

// --- Extract payload (3rd CSV field) after the '=' sign.
// Works for +RCV=, +RCW=, +SCV=, +RECV=, etc., or bare payload lines.
String extractPayload(const String& line) {
  // If there's an '=', parse FROM,LEN,DATA,... after it
  int eq = line.indexOf('=');
  if (eq >= 0) {
    // Substring after '='
    String after = line.substring(eq + 1);  // "from,len,DATA,rssi,snr"
    // Find commas to locate DATA (3rd field)
    int c1 = after.indexOf(',');                // after FROM
    if (c1 < 0) return "";
    int c2 = after.indexOf(',', c1 + 1);        // after LEN
    if (c2 < 0) return "";
    int c3 = after.indexOf(',', c2 + 1);        // after DATA (may be -1)

    int start = c2 + 1;
    int end   = (c3 >= 0) ? c3 : after.length();
    if (start >= end) return "";

    String data = after.substring(start, end);
    data.trim();
    return data;
  }

  // Otherwise assume the line itself IS the payload
  String data = line;
  data.trim();
  return data;
}

// --- Determine which LEDs to light based on payload content ---
// Multiple LEDs can be ON simultaneously (e.g., "BW" -> Blue + White)
void handleMessage(String msg) {
  msg.toUpperCase();  // normalize
  allOff();

  bool any = false;

  if (msg.indexOf('R') >= 0) { digitalWrite(LED_R, HIGH); any = true; }
  if (msg.indexOf('Y') >= 0) { digitalWrite(LED_Y, HIGH); any = true; }
  if (msg.indexOf('G') >= 0) { digitalWrite(LED_G, HIGH); any = true; }
  if (msg.indexOf('B') >= 0) { digitalWrite(LED_B, HIGH); any = true; }
  if (msg.indexOf('W') >= 0) { digitalWrite(LED_W, HIGH); any = true; }

  if (any) {
    Serial.print("[LED] ON: ");
    if (digitalRead(LED_R)) Serial.print("R ");
    if (digitalRead(LED_Y)) Serial.print("Y ");
    if (digitalRead(LED_G)) Serial.print("G ");
    if (digitalRead(LED_B)) Serial.print("B ");
    if (digitalRead(LED_W)) Serial.print("W ");
    Serial.println();
  } else {
    Serial.println("[LED] No R/Y/G/B/W in payload — all OFF");
  }
}

// --- Turn all LEDs off ---
void allOff() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(LED_W, LOW);
}
