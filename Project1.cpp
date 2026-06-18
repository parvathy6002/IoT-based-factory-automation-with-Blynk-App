/******************** BLYNK DETAILS ********************/
#define BLYNK_TEMPLATE_ID "TMPL3UmNaGkE3"
#define BLYNK_TEMPLATE_NAME "Span"
#define BLYNK_AUTH_TOKEN "3BNIfOHw4Mg7n841oo2QKN3DDhX_93Fj"

/******************** LIBRARIES ********************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

/******************** WIFI DETAILS ********************/
char ssid[] = "tharoola";
char pass[] = "anakharao";

/******************** PIN DEFINITIONS ********************/
#define DHTPIN 4
#define DHTTYPE DHT11

#define GAS_PIN 34
#define PROX_PIN 35   // SN04-N (input-only pin)

#define MOTOR_EN 25
#define MOTOR_IN1 26
#define MOTOR_IN2 27

/******************** BLYNK VIRTUAL PINS ********************/
#define VPIN_TEMP V0
#define VPIN_HUM V1
#define VPIN_GAS V2
#define VPIN_MOTOR V6

/******************** OBJECTS ********************/
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

/******************** VARIABLES ********************/
bool conveyorPower = false;
bool motorPaused = false;
unsigned long motorStopStart = 0;
const unsigned long STOP_DURATION = 5000; // 5 seconds

/******************** MOTOR FUNCTIONS ********************/
void motorON()
{
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_EN, HIGH);
}

void motorOFF()
{
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_EN, LOW);
}

/******************** SENSOR READING ********************/
void readSensors()
{
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);

  if (!isnan(temperature) && !isnan(humidity))
  {
    Blynk.virtualWrite(VPIN_TEMP, temperature);
    Blynk.virtualWrite(VPIN_HUM, humidity);
  }

  Blynk.virtualWrite(VPIN_GAS, gasValue);
}

/******************** PROXIMITY + MOTOR LOGIC ********************/
void proximityCheck()
{
  int metalDetected = digitalRead(PROX_PIN); // ACTIVE LOW sensor

  if (conveyorPower)
  {
    // Default → motor runs continuously
    if (!motorPaused)
    {
      motorON();
    }

    // Metal detected → stop motor for 5 seconds
    if (metalDetected == LOW && !motorPaused)
    {
      motorOFF();
      motorPaused = true;
      motorStopStart = millis();
    }

    // After 5 seconds → resume continuous rotation
    if (motorPaused && millis() - motorStopStart >= STOP_DURATION)
    {
      motorPaused = false;
      motorON();
    }
  }
  else
  {
    motorOFF();
    motorPaused = false;
  }
}

/******************** BLYNK BUTTON ********************/
BLYNK_WRITE(VPIN_MOTOR)
{
  conveyorPower = param.asInt();

  if (conveyorPower)
    motorON();
  else
    motorOFF();
}

/******************** SETUP ********************/
void setup()
{
  Serial.begin(9600);

  pinMode(GAS_PIN, INPUT);
  pinMode(PROX_PIN, INPUT_PULLUP); // 🔥 REQUIRED for SN04-N

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);

  motorOFF();
  dht.begin();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(2000L, readSensors);
  timer.setInterval(200L, proximityCheck);
}

/******************** LOOP ********************/
void loop()
{
  Blynk.run();
  timer.run();
}


