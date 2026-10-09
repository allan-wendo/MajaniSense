#include <SPI.h>
#include <LoRa.h>

// PIN MAPPING FOR PHYSICAL WIRING
#define LORA_MISO    20   // ESP32-C6 SPI MISO
#define LORA_MOSI    19   // ESP32-C6 SPI MOSI
#define LORA_SCK     21   // ESP32-C6 SPI SCK
#define LORA_CS      18   // RFM96 NSS / Chip Select
#define LORA_RST     8   // RFM96 Reset pin
#define LORA_DIO0    7   // RFM96 DIO0 (Interrupt pin)

// Set your local frequency (RFM96 is typically 433MHz)
#define LORA_FREQ    433E6 

void setup() {
  // Initialize Serial Monitor
  Serial.begin(921600);
  while (!Serial); // Wait for serial port to connect
  
  Serial.println("\n--- ESP32-C6 RFM96 LoRa Receiver ---");

  // Initialize custom SPI pins for the ESP32-C6
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // Override default library pins
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  // Attempt to initialize the RFM96 module
  Serial.print("Starting LoRa Receiver at ");
  Serial.print(LORA_FREQ / 1E6);
  Serial.println(" MHz...");

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("❌ LoRa Initialization Failed!");
    while (1); // Halt execution
  }

  Serial.println("🎉 Receiver Initialized Successfully! Listening for packets...");
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Received a packet
    Serial.print("📦 Received packet '");

    // Read packet payload
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    // Print RSSI (Received Signal Strength Indicator) of packet
    Serial.print("' with RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
