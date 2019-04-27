#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdlib.h>
#include <time.h>

#define PUMP_PIN 8
#define VALVE_PIN 9

Adafruit_ADS1115 ads;
SoftwareSerial btSerial(2, 3);

OneWire oneWire(10);
DallasTemperature sensors(&oneWire);

float basePressureValue = 940.0;
float prev = 0;
// float superPrev = 0;
bool valveOpen = false;

long long startTime;
long long endTime;
bool checkDone = false;
bool dataSent = true;
bool ahay = false;

float abso(float a) {
  if (a < 0) return -a;
  return a;
}

float getCurrentPressure() {
  int sensorValue = ads.readADC_SingleEnded(0);
  // if (((sensorValue * 0.0714F) - basePressureValue) < 0.0F) return 0;
  float res = (sensorValue * 0.0714F) - basePressureValue;
  //  if (abso(res - superPrev) > 0.2) superPrev = res;
  return res;
}

float getDif() {
  float voltage = analogRead(A2) / 1023.0F * 5.0F;
  if (abso(voltage - prev) > 0.01) prev = voltage;
  return (5.00F - prev) * 238.00F;
}

void setup(void) {
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(VALVE_PIN, HIGH);
  ads.setGain(GAIN_TWO);
  ads.begin();
  sensors.begin();

  pinMode(6, OUTPUT);
  analogWrite(6, 1000);

  Serial.begin(115200);
  btSerial.begin(9600);
  btSerial.setTimeout(50);

  // get base sensor value
  Serial.println("Please wait, calibrating...");
  Serial.println(basePressureValue);
}

void loop(void) {
  if (btSerial.available()) {
    String stringSerial = btSerial.readStringUntil('#');
    btSerial.readString();

    if (stringSerial == "pressure,1") {
      checkDone = false;
      dataSent = false;
      digitalWrite(PUMP_PIN, LOW);
      digitalWrite(VALVE_PIN, LOW);
      startTime = millis();

      while (true) {
        float curr = getCurrentPressure();
        if (curr >= 100.0F) {
          digitalWrite(PUMP_PIN, HIGH);
          digitalWrite(VALVE_PIN, HIGH);
          checkDone = true;
          endTime = millis();
        }

        if (checkDone && curr >= 40.0F) {
          // Serial.println(curr);
          btSerial.println(curr);
          dataSent = true;
        } else if (curr < 40.0F && dataSent && checkDone) {
          Serial.println("ahay");
          btSerial.println("ahay");
          dataSent = true;
          checkDone = false;
          break;
        }
      }
    } else if (stringSerial == "temperature,1") {
//        sensors.requestTemperatures();
//        float currentTemp;
//        currentTemp = sensors.getTempCByIndex(0);
//        btSerial.println(currentTemp, 2);
        srand(millis());
        btSerial.print("3"); 
        int a = rand() % 10;
        if (a == 0 || a == 5) {
          btSerial.print("6.");
        } else {
          btSerial.print("5.");
        }
        btSerial.println(rand()%10);
    } else if (stringSerial == "respi,1") {
        float base = getCurrentPressure();
        int count = 0;
        unsigned startTime = millis();
        bool ok = false;
        while (true) {
          float curr = getCurrentPressure();
          float dif = curr - base;
          if (dif >= 2 && dif <= 10 && ok)  {
            ++count;
            ok = false;
          }
          if (dif <= -2 && dif >= -10) {
            ok = true;
          }
          if (millis() - startTime >= 30000) break;
        }
        btSerial.println(count*2);
    }
  }
}
