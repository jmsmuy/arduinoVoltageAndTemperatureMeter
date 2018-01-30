#include "SevSeg.h"
#include <EEPROM.h>

// which analog pin to connect
#define THERMISTORPIN A5
// which analog pin to connect
#define THERMISTORPIN_2 A3
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 1
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000

#define BUTTON_THRESHOLD 500

#define SERIAL_ENABLED 0

#define TEMPERATURE 1
#define VOLTAGE 2

#define BUTTON_PIN 11
#define THERMISTOR_ACTIVE_PIN 12

#define LAST_MODE_ADDR 0

#define TEMPERATURE_OFFSET_FIX -5

#define DEFAULT_INTERVAL 100
#define ALARM_INTERVAL 500

#define MIN_CAR_VOLTAGE 12.9
#define MAX_CAR_VOLTAGE 14.5

#define ALARM_START_OFFSET 180000

uint16_t samples[NUMSAMPLES];

SevSeg sevseg;
int mode = TEMPERATURE;

byte numDigits = 3;
byte digitPins[] = {2, 1, 0};
byte segmentPins[] = {5, 3, 9, 7, 6, 4, 10, 8};
bool resistorsOnSegments = false;
byte hardwareConfig = COMMON_CATHODE;
bool updateWithDelays = true;
bool leadingZeros = false;
bool buttonEnabled = true;
long lastButtonChange;
bool voltageAlarm = false;
long currentTime;
long lastUpdate;
int intervalBetweenUpdates = DEFAULT_INTERVAL;
bool alarmCheckEnabled = false;

void setupDisplay() {
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(100);
}

void setup() {
  analogReference(EXTERNAL);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(THERMISTOR_ACTIVE_PIN, OUTPUT);
  digitalWrite(THERMISTOR_ACTIVE_PIN, HIGH);

  lastButtonChange = millis();
  currentTime = lastButtonChange;

  mode = EEPROM.read(LAST_MODE_ADDR);

  if (SERIAL_ENABLED) {
    Serial.begin(9600);
  }
  setupDisplay();

}

boolean checkVoltage(float voltage) {
  return voltage < MIN_CAR_VOLTAGE || voltage > MAX_CAR_VOLTAGE;
}

float getVoltage() {
  return analogRead(A4) / 53.42f;
}

int getTemperature() {
  digitalWrite(THERMISTOR_ACTIVE_PIN, LOW);
  float average = analogRead(THERMISTORPIN);
  digitalWrite(THERMISTOR_ACTIVE_PIN, HIGH);
  if (SERIAL_ENABLED) {
    Serial.print("raw_temp:");
    Serial.println(average);
  }
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  if (SERIAL_ENABLED) {
    Serial.print("raw_temp2:");
    Serial.println(average);
  }
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  return steinhart + TEMPERATURE_OFFSET_FIX;
}

void loop() {
  sevseg.refreshDisplay();

  currentTime = millis();
  if (currentTime - lastUpdate >= intervalBetweenUpdates) {

    if (currentTime > ALARM_START_OFFSET && !alarmCheckEnabled) {
      alarmCheckEnabled = true;
    }

    float voltage = getVoltage();
    int temperature = getTemperature();

    if (alarmCheckEnabled) {
      voltageAlarm = checkVoltage(voltage);
    }

    if (voltageAlarm) {
      mode = VOLTAGE;
      sevseg.setNumber(voltage, 1);
      if (SERIAL_ENABLED) {
        Serial.println("Alarmm! DANGEROUS VOLTAGE");
      }
      intervalBetweenUpdates = ALARM_INTERVAL;
      delay(ALARM_INTERVAL);
    } else if (mode == TEMPERATURE) {
      sevseg.setTemperature(temperature, 0);
      if (SERIAL_ENABLED) {
        Serial.println(temperature);
      }
      intervalBetweenUpdates = DEFAULT_INTERVAL;
    } else {
      sevseg.setNumber(voltage, 1);
      if (SERIAL_ENABLED) {
        Serial.println(voltage);
      }
      intervalBetweenUpdates = DEFAULT_INTERVAL;
    }

    currentTime = millis();
    lastUpdate = currentTime;

    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState && buttonEnabled && !voltageAlarm) {
      if (currentTime - lastButtonChange > BUTTON_THRESHOLD) {
        buttonEnabled = false;
        if (mode == TEMPERATURE) {
          mode = VOLTAGE;
        } else {
          mode = TEMPERATURE;
        }
        EEPROM.write(LAST_MODE_ADDR, mode);
      }
    } else {
      lastButtonChange = currentTime;
      buttonEnabled = true;
    }
  }
}

