/*
=========================================================
SMART GREENHOUSE SYSTEM
Arduino Uno
Author: Allan
=========================================================
*/

#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

//==================== PIN DEFINITIONS ====================

// Sensors
#define SOIL_PIN A0
#define LDR_PIN A1
#define MQ135_PIN A2
#define DHT_PIN 2

// Actuators
#define FAN_RELAY 3
#define PUMP_RELAY 7
#define SERVO_RELAY 8
#define SERVO_PIN 9
#define LED_RELAY 12

// GSM
#define GSM_RX 10
#define GSM_TX 11

//==================== DHT ====================

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

//==================== SERVO ====================

Servo greenhouseServo;

//==================== GSM ====================

SoftwareSerial sim800(GSM_RX, GSM_TX);

//==================== SENSOR VARIABLES ====================

float temperature = 0;
float humidity = 0;

int soilRaw = 0;
int soilPercent = 0;

int ldrValue = 0;

int mqRaw = 0;
int airPPM = 0;

//==================== SYSTEM STATES ====================

bool pumpState = false;
bool ledState = false;
bool servoOpen = false;
bool fanState = false;          // <-- added

//==================== ALERT FLAGS ====================

bool tempAlert = false;
bool humidityAlert = false;
bool soilAlert = false;
bool lightAlert = false;
bool airAlert = false;

//==================== FUNCTION PROTOTYPES ====================

void readSensors();
void controlPump();
void controlLED();
void controlServo();
void controlFan();              // <-- added
void checkAlerts();
void sendSMS(String message);
void printReadings();
void moveServo(bool open);

//========================================================

void setup()
{
    Serial.begin(9600);
    sim800.begin(9600);

    dht.begin();

    greenhouseServo.attach(SERVO_PIN);

    pinMode(FAN_RELAY, OUTPUT);     // <-- added
    pinMode(PUMP_RELAY, OUTPUT);
    pinMode(SERVO_RELAY, OUTPUT);
    pinMode(LED_RELAY, OUTPUT);

    // Relay module is ACTIVE LOW
    digitalWrite(FAN_RELAY, HIGH);  // <-- added (fan off at start)
    digitalWrite(PUMP_RELAY, HIGH);
    digitalWrite(SERVO_RELAY, HIGH);
    digitalWrite(LED_RELAY, HIGH);

    greenhouseServo.write(0);

    Serial.println("=================================");
    Serial.println(" SMART GREENHOUSE SYSTEM ");
    Serial.println("=================================");
}

void loop()
{
    readSensors();

    controlPump();
    controlLED();
    controlServo();
    controlFan();               // <-- added

    Serial.println("================================");

    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity : ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Soil Moisture : ");
    Serial.print(soilPercent);
    Serial.print("%   Pump : ");
    Serial.println(pumpState ? "ON" : "OFF");

    Serial.print("Light Intensity : ");
    Serial.print(ldrValue);
    Serial.print("   LED : ");
    Serial.println(ledState ? "ON" : "OFF");

    Serial.print("Air Quality : ");
    Serial.print(airPPM);
    Serial.println(" ppm");

    Serial.print("Servo : ");
    Serial.println(servoOpen ? "OPEN" : "CLOSED");

    Serial.print("Fan : ");                 // <-- added
    Serial.println(fanState ? "ON" : "OFF");

    Serial.println("================================");

    delay(2000);
}

/*
=========================================================
SENSORS MODULE
=========================================================
*/

void readSensors()
{
    // Read DHT11
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Error reading DHT11!");
        return;
    }

    // Read Soil Moisture Sensor
    soilRaw = analogRead(SOIL_PIN);

    // Initial calibration values
    // soilPercent = map(soilRaw, 1023, 350, 0, 100);
    // soilPercent = constrain(soilPercent, 0, 100);

    soilPercent = constrain(map(soilRaw, 1023, 350, 0, 100),0,100);
  

    // Read LDR
    ldrValue = analogRead(LDR_PIN);

    // Read MQ135
    mqRaw = analogRead(MQ135_PIN);

    // Approximate conversion to ppm
    airPPM = map(mqRaw, 0, 1023, 0, 1000);
}

/*
=========================================================
ACTUATORS MODULE
Controls:
1. Pump
2. LED
3. Servo
4. Fan
=========================================================
*/


//---------------------------------------------------------
// Pump Control
//---------------------------------------------------------
void controlPump()
{
    // Pump ON if soil moisture is below 50%
    if (soilPercent < 50)
    {
        digitalWrite(PUMP_RELAY, LOW);      // Active LOW relay
        pumpState = true;
    }

    // Pump OFF once soil moisture reaches 60%
    else if (soilPercent >= 60)
    {
        digitalWrite(PUMP_RELAY, HIGH);     // Active LOW relay
        pumpState = false;
    }
}


//---------------------------------------------------------
// LED Control
//---------------------------------------------------------
void controlLED()
{
    // Turn LED ON when light intensity is low
    if (ldrValue < 280)
    {
        digitalWrite(LED_RELAY, LOW);       // Active LOW relay
        ledState = true;
    }

    // Turn LED OFF otherwise
    else
    {
        digitalWrite(LED_RELAY, HIGH);      // Active LOW relay
        ledState = false;
    }
}


//---------------------------------------------------------
// Servo Movement Function
//---------------------------------------------------------
void moveServo(bool open)
{
    // Do nothing if servo is already in required position
    if (servoOpen == open)
        return;

    // Power the servo through the relay
    digitalWrite(SERVO_RELAY, LOW);
    delay(200);

    if (open)
    {
        greenhouseServo.write(180);
        Serial.println("Servo OPENED");
    }
    else
    {
        greenhouseServo.write(0);
        Serial.println("Servo CLOSED");
    }

    delay(800);

    // Remove power from servo
    digitalWrite(SERVO_RELAY, HIGH);

    servoOpen = open;
}


//---------------------------------------------------------
// Servo Priority Logic
//---------------------------------------------------------
void controlServo()
{
    //=========================
    // Priority 1: Air Quality
    //=========================

    if (airPPM < 420)
    {
        moveServo(true);      // Open vent
        return;
    }

    if (airPPM > 450)
    {
        moveServo(false);     // Close vent
        return;
    }


    //=========================
    // Priority 2: Temperature
    //=========================

    if (temperature > 23)
    {
        moveServo(true);      // Open sprinkler valve
        return;
    }

    if (temperature <= 21)
    {
        moveServo(false);     // Close sprinkler valve
        return;
    }


    //=========================
    // Priority 3: Humidity
    //=========================

    if (humidity < 65)
    {
        moveServo(true);      // Open sprinkler valve
    }
    else
    {
        moveServo(false);     // Close sprinkler valve
    }
}


//---------------------------------------------------------
// Fan Control (NEW)
//---------------------------------------------------------
void controlFan()
{
    // Fan ON when temperature is high (cooling needed)
    if (temperature > 23)
    {
        digitalWrite(FAN_RELAY, LOW);       // Active LOW relay
        fanState = true;
    }
    // Fan OFF when temperature is cool enough
    else if (temperature <= 21)
    {
        digitalWrite(FAN_RELAY, HIGH);      // Active LOW relay
        fanState = false;
    }
    // Between 21 °C and 23 °C the fan keeps its previous state
    // (hysteresis – same idea used by the pump)
}


/*
=========================================================
ALERTS MODULE
=========================================================
*/

void checkAlerts()
{
    //==================== TEMPERATURE ====================

    if (temperature > 23)
    {
        if (!tempAlert)
        {
            sendSMS("SMART GREENHOUSE ALERT\n\nTemperature HIGH\nTemperature: " +
                    String(temperature) +
                    " C\nAction: Sprinkler OPENED");

            tempAlert = true;
        }
    }
    else if (temperature >= 21 && temperature <= 23)
    {
        tempAlert = false;
    }


    //==================== HUMIDITY ====================

    if (humidity < 65)
    {
        if (!humidityAlert)
        {
            sendSMS("SMART GREENHOUSE ALERT\n\nHumidity LOW\nHumidity: " +
                    String(humidity) +
                    " %\nAction: Sprinkler OPENED");

            humidityAlert = true;
        }
    }
    else
    {
        humidityAlert = false;
    }


    //==================== SOIL MOISTURE ====================

    if (soilPercent < 50)
    {
        if (!soilAlert)
        {
            sendSMS("SMART GREENHOUSE ALERT\n\nSoil Moisture LOW\nSoil Moisture: " +
                    String(soilPercent) +
                    " %\nAction: Pump ACTIVATED");

            soilAlert = true;
        }
    }
    else
    {
        soilAlert = false;
    }


    //==================== LIGHT ====================

    if (ldrValue < 300)
    {
        if (!lightAlert)
        {
            sendSMS("SMART GREENHOUSE ALERT\n\nLight Intensity LOW\nAction: LED TURNED ON");

            lightAlert = true;
        }
    }
    else
    {
        lightAlert = false;
    }


    //==================== AIR QUALITY ====================

    if (airPPM < 350 || airPPM > 450)
    {
        if (!airAlert)
        {
            sendSMS("SMART GREENHOUSE ALERT\n\nAir Quality Outside Optimum Range\nAir Quality: " +
                    String(airPPM) +
                    " ppm");

            airAlert = true;
        }
    }
    else
    {
        airAlert = false;
    }
}