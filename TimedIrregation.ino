// Based on DS3231_Serial_Hard
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
//
// Arduino Uno/2009:
// ----------------------
// DS3231:  SDA pin   -> Arduino Analog 4 or the dedicated SDA pin
//          SCL pin   -> Arduino Analog 5 or the dedicated SCL pin
//

#include <DS3231.h>
#include <SPI.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Declare a Time-data structure
Time  t;
int wateringMonths[] = { 5, 6, 7, 8, 9, 10 }; // JAUNUARY=1, ..., DECEMBER=12
int wateringDays[] = { 1, 3, 5 }; // Monday=1, TUESDAY=2, Wednessday=3, THURSDAY=4, Friday=5, SATURDAY=6 and SUNDAY=7
int wateringHours[] = { 19 }; // water these hours. 24 hour format
int wateringMins[] = {  }; // water irregation these minutes. If minutes left empty, will run for the whole hour
//int wateringMins[] = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 30, 32, 35, 37, 40, 41, 42, 43, 44, 46, 50, 51, 53, 55, 57 }; // water irregation these minutes. If minutes left empty, will run for the whole hour

const int relayPin1 = 3;
const int relayPin2 = 2;
const int CSPin     = 4;

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);

  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);

  // Initialize the rtc object
  rtc.begin();
  t = rtc.getTime();
  logInfo(t);

  // The following lines can be uncommented to set the date and time
  /*
    rtc.setDOW(SUNDAY);     // Set Day-of-Week to SUNDAY
    rtc.setTime(1, 23, 0);     // Set the time to 12:00:00 (24hr format)
    rtc.setDate(9, 6, 2019);   // Set the date to January 1st, 2014
  */
}

void loop()
{
  // Get data from the DS3231
  t = rtc.getTime();
  
  // engage/disengage water pump
  if (isWateringMonth() && isWateringDay() && isWateringHour() && isWateringMinute()) {
    if (digitalRead(relayPin1) == HIGH) {
      digitalWrite(relayPin1, LOW);
      logInfo(t);
    }
  } else {
    if (digitalRead(relayPin1) == LOW) {
      digitalWrite(relayPin1, HIGH);
      logInfo(t);
    }
  }

  delay (1000);
}

bool isWateringMonth() {
  for (int i = 0; i < sizeof(wateringMonths) / sizeof (int); i++) {
    if (wateringMonths[i] == t.mon) {
      return true;
    }
  }
  return false;
}
bool isWateringDay() {
  for (int i = 0; i < sizeof(wateringDays) / sizeof (int); i++) {
    if (wateringDays[i] == t.dow) {
      return true;
    }
  }
  return false;
}
bool isWateringHour() {
  for (int i = 0; i < sizeof(wateringHours) / sizeof (int); i++) {
    if (wateringHours[i] == t.hour) {
      return true;
    }
  }
  return false;
}
bool isWateringMinute() {
  if (sizeof(wateringMins) == 0) {
    return true;
  }
  else {
    for (int i = 0; i < sizeof(wateringMins) / sizeof (int); i++) {
      if (wateringMins[i] == t.min) {
        return true;
      }
    }
  }
  return false;
}
void logInfo(Time& t) {
  char buff[32];
  sprintf(buff, "%02d,%02d,%02d,%02d,%02d,%02d,%01d", t.year, t.mon, t.date, t.hour, t.min, t.sec, digitalRead(relayPin1));
  Serial.println(buff);
}
