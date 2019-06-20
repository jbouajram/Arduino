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
#include <SD.h>
#include "RTClib.h"

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Declare a Time-data structure
Time  t;
// Declare an SD card file
File myFile;

String logFile = "waterLog.txt";


int wateringDays[] = { 2, 4, 6, 7 }; // Monday=1, TUESDAY=2, Wednessday=3, THURSDAY=4, Friday=5, SATURDAY=6 and SUNDAY=7
int wateringHours[] = { 1, 2, 3, 10, 11, 12, 13, 17, 18, 19, 20, 21 }; // water these hours. 24 hour format
//int wateringMins[] = {  }; // water irregation these minutes. If minutes left empty, will run for the whole hour
int wateringMins[] = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 30, 32, 35, 37, 40, 41, 44, 46, 50, 51, 53, 55, 57 }; // water irregation these minutes. If minutes left empty, will run for the whole hour

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
String buffer;

// variables to hold the parsed data
int logRecord[8] = {0}; //logRecord ( year, mon, date, hour, min, sec, relayStatus, water interval in days, watering hour);
const int WATERINTERVAL = 3;
const int WATERHOUR = 19;

char rx_byte = 0;
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

  // The following lines can be uncommented to set the date and time
  /*
    rtc.setDOW(SUNDAY);     // Set Day-of-Week to SUNDAY
    rtc.setTime(1, 23, 0);     // Set the time to 12:00:00 (24hr format)
    rtc.setDate(9, 6, 2019);   // Set the date to January 1st, 2014
  */
  initSDCard();  
}

void loop()
{
  // reset log file by entering 'R' on serial monitor
  if (Serial.available() > 0) {    // is a character available?
    rx_byte = Serial.read();       // get the character
  
    // check if a reset command was received
    if (rx_byte == 'R') {
      resetLogFile();
      Serial.println(rx_byte);
    }
  }

  // Get data from the DS3231
  t = rtc.getTime();
  // engage/disengage water pump
  int waterSchedule = 0;
  for (int i = 0; i < sizeof(wateringDays) / sizeof (int); i++) {
    if (wateringDays[i] == t.dow) {
      waterSchedule = 1;
      break;
    }
  }
  if (waterSchedule) {
    waterSchedule = 0;
    for (int i = 0; i < sizeof(wateringHours) / sizeof (int); i++) {
      if (wateringHours[i] == t.hour) {
        waterSchedule = 1;
        break;
      }
    }
  }
  if (waterSchedule) {
    waterSchedule = 0;
    if (sizeof(wateringMins) == 0) {
      waterSchedule = 1;
    }
    else {
      for (int i = 0; i < sizeof(wateringMins) / sizeof (int); i++) {
        if (wateringMins[i] == t.min) {
          waterSchedule = 1;
          break;
        }
      }
    }
  }

  if (waterSchedule) {
    if (digitalRead(relayPin1) == HIGH) {
      digitalWrite(relayPin1, LOW);
      writeLogFile(t);
    }
  } else {
    if (digitalRead(relayPin1) == LOW) {
      digitalWrite(relayPin1, HIGH);
      writeLogFile(t);
    }
  }

  delay (1000);
}
void writeLogFile(char buff[32]) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(logFile, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println(buff);
    myFile.println(buff);
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening log file ");
    Serial.println(logFile);
  }
}
void writeLogFile(Time& t) {
  char buff[32];
  sprintf(buff, "%02d,%02d,%02d,%02d,%02d,%02d,%01d,%01d,%01d", t.year, t.mon, t.date, t.hour, t.min, t.sec, digitalRead(relayPin1), WATERINTERVAL, WATERHOUR);
  writeLogFile(buff);
}

void resetLogFile() {
  SD.remove(logFile);
  myFile = SD.open(logFile, FILE_WRITE);
  myFile.close();
  Time pastTime;
  writeLogFile(pastTime);
}

void initSDCard() {
  // Initialize the SD card
  if (!SD.begin(CSPin)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialization done.");
  }

  if (!SD.exists(logFile)) {
    Serial.print(logFile);
    Serial.println(" doesnt exist, will create afresh");
    resetLogFile();
  }

  // open the file for reading:
  myFile = SD.open(logFile);
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(logFile);
  }
  // open the file for reading:
  myFile = SD.open(logFile);
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      buffer = myFile.readStringUntil('\n');
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.print("error opening ");
    Serial.println(logFile);
  }
  parseDate();
  compareDates();
}

void parseDate() {
  char charBuf[buffer.length() + 1];
  buffer.toCharArray(charBuf, buffer.length());
  // Serial.println (charBuf);
  char * pch;
  pch = strtok (charBuf, ",");
  int i = 0;
  while (pch != NULL)
  {
    logRecord[i++] = atoi(pch);
    pch = strtok (NULL, ",");
  }
}
void compareDates() {
  t = rtc.getTime();
  DateTime lastDate(logRecord[0], logRecord[1], logRecord[2], logRecord[3], logRecord[4], logRecord[5]);
  DateTime currentDate( t.year, t.mon, t.date, t.hour, t.min, t.sec);
  //showDate("lastDate", lastDate);
  //showDate("currentDate", currentDate);
  TimeSpan ts1 = lastDate - currentDate;
  //showTimeSpan("time difference = ", ts1);
}
void showTimeSpan(const char* txt, const TimeSpan& ts) {
  Serial.print(txt);
  Serial.print(" ");
  Serial.print(ts.days(), DEC);
  Serial.print(" days ");
  Serial.print(ts.hours(), DEC);
  Serial.print(" hours ");
  Serial.print(ts.minutes(), DEC);
  Serial.print(" minutes ");
  Serial.print(ts.seconds(), DEC);
  Serial.print(" seconds (");
  Serial.print(ts.totalseconds(), DEC);
  Serial.print(" total seconds)");
  Serial.println();
}
void showDate(const char* txt, const DateTime& dt) {
  Serial.print(txt);
  Serial.print(' ');
  Serial.print(dt.year(), DEC);
  Serial.print('/');
  Serial.print(dt.month(), DEC);
  Serial.print('/');
  Serial.print(dt.day(), DEC);
  Serial.print(' ');
  Serial.print(dt.hour(), DEC);
  Serial.print(':');
  Serial.print(dt.minute(), DEC);
  Serial.print(':');
  Serial.print(dt.second(), DEC);

  Serial.print(" = ");
  Serial.print(dt.unixtime());
  Serial.print("s / ");
  Serial.print(dt.unixtime() / 86400L);
  Serial.print("d since 1970");

  Serial.println();
}
