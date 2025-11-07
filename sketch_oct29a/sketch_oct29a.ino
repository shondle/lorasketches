#include <SoftwareSerial.h>
SoftwareSerial lora(10, 11); // RX, TX

// Optional: quick helper to build the correct AT+SEND with length = payload size
void sendPayload(uint8_t destAddr, const String& payload) {
  String cmd = "AT+SEND=";
  cmd += String(destAddr);
  cmd += ",";
  cmd += String(payload.length());   // length must match payload bytes
  cmd += ",";
  cmd += payload;
  sendCommand(cmd);
}

void setup() {
  Serial.begin(115200);       // For your Serial Monitor
  lora.begin(115200);         // Match this baud to the module
  Serial.println("=== LoRa Transmitter Test ===");

  delay(1000); // Give the module time to boot

  // --- Basic communication test ---
  Serial.println("[TEST] Sending 'AT' command...");
  lora.println("AT");
  delay(300);
  readLoRaResponse();

  // --- Configure module parameters ---
  Serial.println("[SETUP] Configuring address, network, and band...");
  sendCommand("AT+ADDRESS=1");
  sendCommand("AT+NETWORKID=5");
  sendCommand("AT+BAND=915000000");

  Serial.println("[SETUP] Checking parameters...");
  sendCommand("AT+ADDRESS?");
  sendCommand("AT+NETWORKID?");
  sendCommand("AT+BAND?");

  Serial.println("[INFO] Transmitter ready. Sending test messages...");
  delay(1000);
}

void loop() {
  // Destination is your receiver at address 2
  const uint8_t DEST = 2;

  // Single-LED tests (R=pin6, Y=5, G=4, B=3, W=2 on the receiver)
  sendPayload(DEST, "R"); delay(1500);
  sendPayload(DEST, "Y"); delay(1500);
  sendPayload(DEST, "G"); delay(1500);
  sendPayload(DEST, "B"); delay(1500);
  sendPayload(DEST, "W"); delay(1500);

  // Multi-LED combos (turn on multiple LEDs simultaneously on the receiver)
  sendPayload(DEST, "RY");    delay(1500);  // Red + Yellow
  sendPayload(DEST, "GW");    delay(1500);  // Green + White
  sendPayload(DEST, "BW");    delay(1500);  // Blue + White
  sendPayload(DEST, "RGB");   delay(1500);  // Red + Green + Blue
  sendPayload(DEST, "RYG");   delay(1500);  // Red + Yellow + Green
  sendPayload(DEST, "YBW");   delay(1500);  // Yellow + Blue + White
  sendPayload(DEST, "RYGBW"); delay(2000);  // All five on

  // Optional clear between patterns
  sendPayload(DEST, "");      delay(1000);  // If receiver treats empty as "no match", LEDs go off
  // Or explicitly send a non-matching token if you prefer:
  // sendPayload(DEST, "CLEAR"); delay(1000);
}

// --- Utility functions ---

// Sends a command and reads back the reply
void sendCommand(String cmd) {
  Serial.print(">> "); Serial.println(cmd);
  lora.println(cmd);
  delay(300);
  readLoRaResponse();
}

// Reads any available text from the LoRa module
void readLoRaResponse() {
  while (lora.available()) {
    char c = lora.read();
    Serial.write(c);
  }
  Serial.println();
}
