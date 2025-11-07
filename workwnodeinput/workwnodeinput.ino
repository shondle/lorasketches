#include <SoftwareSerial.h>
SoftwareSerial lora(10, 11); // RX, TX  (keep these; avoid using 10/11 elsewhere)

// ----- Sensor config -----
const int N_SENSORS  = 5;
const int A_PINS[N_SENSORS]     = {A0, A1, A2, A3, A4};   // analog sense pins
const int DRIVE_PINS[N_SENSORS] = {2,  3,  4,  5,  6 };   // excite each sensor separately
const char LETTERS[N_SENSORS]   = {'W','B','Y','G','R'};  // 1→W, 2→B, 3→Y, 4→G, 5→R

const int N_SAMPLES = 8;
int THRESHOLD = 10;  // start low; tune after first tests

// ----- LoRa dest -----
const uint8_t DEST_ADDR = 2;

// ----------------- Utilities -----------------
void sendCommand(String cmd) {
  Serial.print(">> "); Serial.println(cmd);
  lora.println(cmd);
  delay(300);
  // Read module echo/reply (non-blocking-ish)
  while (lora.available()) {
    Serial.write(lora.read());
  }
  Serial.println();
}

void readLoRaResponse() {
  while (lora.available()) {
    char c = lora.read();
    Serial.write(c);
  }
  Serial.println();
}

// Helper to format AT+SEND with correct length
void sendPayload(uint8_t destAddr, const String& payload) {
  String cmd = "AT+SEND=";
  cmd += String(destAddr);
  cmd += ",";
  cmd += String(payload.length());
  cmd += ",";
  cmd += payload;
  sendCommand(cmd);
}

// Excite one sensor, average N_SAMPLES, then remove DC to reduce corrosion
int readWaterRawOne(int i) {
  digitalWrite(DRIVE_PINS[i], HIGH);
  delay(3);  // settle
  long acc = 0;
  for (int k = 0; k < N_SAMPLES; k++) {
    acc += analogRead(A_PINS[i]);
  }
  digitalWrite(DRIVE_PINS[i], LOW);
  return (int)(acc / N_SAMPLES);  // 0..1023
}

// ----------------- Arduino lifecycle -----------------
void setup() {
  Serial.begin(115200);
  lora.begin(115200);

  // Drive pins as outputs, idle LOW (no DC when idle)
  for (int i = 0; i < N_SENSORS; i++) {
    pinMode(DRIVE_PINS[i], OUTPUT);
    digitalWrite(DRIVE_PINS[i], LOW);
  }

  Serial.println("=== LoRa Transmitter: 5-input water multiplexer ===");
  delay(1000);

  // Basic communication test
  Serial.println("[TEST] Sending 'AT' command...");
  lora.println("AT");
  delay(300);
  readLoRaResponse();

  // Configure module
  Serial.println("[SETUP] Configuring address, network, band...");
  sendCommand("AT+ADDRESS=1");
  sendCommand("AT+NETWORKID=5");
  sendCommand("AT+BAND=915000000");

  Serial.println("[SETUP] Checking parameters...");
  sendCommand("AT+ADDRESS?");
  sendCommand("AT+NETWORKID?");
  sendCommand("AT+BAND?");

  Serial.println("[INFO] Transmitter ready. Reading 5 sensors and sending letters for any above threshold.");
  delay(500);
}

void loop() {
  String payload = "";

  // Read all sensors; append their letter if above threshold
  for (int i = 0; i < N_SENSORS; i++) {
    int raw = readWaterRawOne(i);

    // Debug print for tuning
    Serial.print("S"); Serial.print(i+1);
    Serial.print(" raw=");
    Serial.print(raw);
    if (raw > THRESHOLD) {
      Serial.print("  -> HIT("); Serial.print(LETTERS[i]); Serial.print(")");
      payload += LETTERS[i];
    }
    Serial.println();
  }

  if (payload.length() == 0) {
    // No hits: send a benign token so receiver clears LEDs
    payload = "0";  // receiver finds no R/Y/G/B/W → all off
  }

  sendPayload(DEST_ADDR, payload);

  // Pace the loop; adjust to your needs
  delay(750);
}
