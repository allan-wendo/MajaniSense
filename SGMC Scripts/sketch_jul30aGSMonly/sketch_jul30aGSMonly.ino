#include <SoftwareSerial.h>

// Configure software serial pins (RX, TX)
SoftwareSerial gsmSerial(10, 11); 

// Configuration variables
const String FARMER_PHONE = "+254769177475"; // Replace with farmer's mobile number with country code
const int SENSOR_PIN = A0;                // Example analog sensor pin (e.g., soil moisture)
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 3600000; // Send update every hour (in milliseconds)

void setup() {
  // Start hardware serial for debugging
  Serial.begin(9600);
  
  // Start software serial for GSM communication
  gsmSerial.begin(9600);
  
  Serial.println("Initializing GSM Module...");
  delay(1000);
  
  // Test communication
  gsmSerial.println("AT"); 
  updateSerial();
  
  // Set SMS text mode
  gsmSerial.println("AT+CMGF=1"); 
  updateSerial();
}

void loop() {
  // Check if it is time to send data
  if (millis() - lastSendTime >= SEND_INTERVAL || lastSendTime == 0) {
    int sensorValue = analogRead(SENSOR_PIN);
    
    // Map or process your sensor data here
    // Example: Convert to percentage for soil moisture
    int moisturePercent = map(sensorValue, 1023, 0, 0, 100); 
    
    String message = "Greenhouse Alert: Soil Moisture is at " + String(moisturePercent) + "%.";
    
    sendSMS(FARMER_PHONE, message);
    lastSendTime = millis();
  }
}

// Function to send SMS
void sendSMS(String phoneNumber, String text) {
  Serial.println("Sending SMS...");
  
  // Configure destination phone number
  gsmSerial.println("AT+CMGS=\"" + phoneNumber + "\"");
  delay(1000);
  
  // Send message content
  gsmSerial.print(text);
  delay(500);
  
  // Send Ctrl+Z (ASCII code 26) to signal end of message
  gsmSerial.write(26); 
  delay(5000); 
  
  Serial.println("SMS Sent!");
  updateSerial();
}

// Helper function to print GSM responses to the Arduino Serial Monitor
void updateSerial() {
  delay(500);
  while (Serial.available()) {
    gsmSerial.write(Serial.read()); // Forward from Serial Monitor to GSM
  }
  while (gsmSerial.available()) {
    Serial.write(gsmSerial.read()); // Forward from GSM to Serial Monitor
  }
}
