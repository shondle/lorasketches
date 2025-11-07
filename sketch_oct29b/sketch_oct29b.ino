#include <SoftwareSerial.h>
SoftwareSerial lora(10, 11); // RX, TX


void setup() {
  Serial.begin(115200);
  lora.begin(115200);
  Serial.println("=== LoRa Receiver Test ===");

  delay(1000);

  sendCommand("AT");
  sendCommand("AT+ADDRESS=2");
  sendCommand("AT+NETWORKID=5");
  sendCommand("AT+BAND=915000000");

  Serial.println("[INFO] Receiver ready.");
}

void loop() {
  readLoRaResponse();
}

// --- Utility functions ---
void sendCommand(String cmd) {
  Serial.print(">> "); Serial.println(cmd);
  lora.println(cmd);
  delay(300);
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
}
