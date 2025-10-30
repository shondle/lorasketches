#include <SoftwareSerial.h>
SoftwareSerial lora(10, 11); // RX, TX

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

  Serial.println("[INFO] Transmitter ready. Sending test messages every 3 seconds...");
  delay(1000);
}

void loop() {
  // Try sending a test packet to receiver at address 2
  sendCommand("AT+SEND=2,5,HELLO");
  delay(3000);  // Wait before next send
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
