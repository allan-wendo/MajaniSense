#include <SPI.h>
#include <LoRa.h>

// Define target pins
const int csPin = 10;          
const int resetPin = 6;       
const int irqPin = 4;          

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender Initializing...");
  LoRa.setPins(csPin, resetPin, irqPin);

  // Replace with your module's frequency (e.g. 433E6, 868E6, 915E6)
  if (!LoRa.begin(433E6)) { 
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  // Optional sync word to isolate network trafficking (0x12-0xFF)
  LoRa.setSyncWord(0xF3); 
  Serial.println("LoRa Initialized successfully.");
}

void loop() {
  // Simulate reading a sensor value
  int sensorValue = analogRead(A0); 
  
  Serial.print("Sending packet: ");
  Serial.println(sensorValue);

  // Send packet
  LoRa.beginPacket();
  LoRa.print("Sensor:");
  LoRa.print(sensorValue);
  LoRa.endPacket();

  delay(5000); // Send data every 5 seconds
}
