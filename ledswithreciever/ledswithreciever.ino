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

    // Extract only the payload; fall back to the whole line if format differs
    String payload = extractPayload(line);
    if (payload.length() == 0) {
      Serial.println("[PARSE] No payload found; LEDs unchanged.");
      continue;
    }

    handleMessage(payload);
  }
}

// --- Extract payload from lines like: +RCV=from,len,DATA,rssi,snr
String extractPayload(const String& line) {
  // Normal form
  if (line.startsWith("+RCV=") || line.startsWith("+SCV=")) {
    // Find commas
    int c1 = line.indexOf(',');                 // after FROM
    if (c1 < 0) return "";
    int c2 = line.indexOf(',', c1 + 1);         // after LEN
    if (c2 < 0) return "";
    int c3 = line.indexOf(',', c2 + 1);         // after DATA (could be -1 if missing)

    int start = c2 + 1;
    int end   = (c3 >= 0) ? c3 : line.length();
    if (start >= end) return "";

    String data = line.substring(start, end);
    data.trim();
    return data;
  }

  // Some firmwares output only the payload
  return line;
}

// --- Determine which LED to light based on message content (payload only) ---
void handleMessage(String msg) {
  msg.toUpperCase();  // normalize

  // Turn all off first
  allOff();

  // Priority: R -> Y -> G -> B -> W (change order if you want different precedence)
  if (msg.indexOf("R") >= 0) {
    digitalWrite(LED_R, HIGH);
    Serial.println("[LED] RED ON (payload contains R)");
  }
  else if (msg.indexOf("Y") >= 0) {
    digitalWrite(LED_Y, HIGH);
    Serial.println("[LED] YELLOW ON (payload contains Y)");
  }
  else if (msg.indexOf("G") >= 0) {
    digitalWrite(LED_G, HIGH);
    Serial.println("[LED] GREEN ON (payload contains G)");
  }
  else if (msg.indexOf("B") >= 0) {
    digitalWrite(LED_B, HIGH);
    Serial.println("[LED] BLUE ON (payload contains B)");
  }
  else if (msg.indexOf("W") >= 0) {
    digitalWrite(LED_W, HIGH);
    Serial.println("[LED] WHITE ON (payload contains W)");
  }
  else {
    Serial.println("[LED] No R/Y/G/B/W found in payload — all OFF");
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
