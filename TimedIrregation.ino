// TimedIrregation.ino
// Based on DS3231_Serial_Hard
//

#include <DS3231.h>
#include <SPI.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Declare a Time-data structure
Time  timeInfo;
Time  manualStartTime;
int wateringMonths[][9] = { { 5, 6, 7, 8, 9, 10 }, { 5, 6, 7, 8, 9, 10 }}; // JAUNUARY=1, ..., DECEMBER=12
int wateringDays[][7] = {{ 7 }, { 5 }}; // Monday=1, TUESDAY=2, Wednessday=3, THURSDAY=4, Friday=5, SATURDAY=6 and SUNDAY=7
int wateringHours[][24] = {{ 19, 20 }, { 18, 19}}; // water these hours. 24 hour format
int wateringMins[] = { }; // water irregation these minutes. If minutes left empty, will run for the whole hour
//int wateringMins[] = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59 }; // water irregation these minutes. If minutes left empty, will run for the whole hour

// Variables will change:
int ledState = LOW;           // the current state of the output pin
int buttonState = HIGH;       // the current reading from the input pin
int lastButtonState = LOW;    // the previous reading from the input pin
int reading = 0;
String comments = "Start program";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool manualSwitchOn = false;    //
bool autoSwitchOn = false;    //
unsigned long stopWatch = 0;    //
const unsigned long irrTimer = 7200000;    // irrigation timer in milliseconds
//

const int relayPin = 3;
//const int relayPin2 = 2;

const int ledPin    = 7;
const int buttonPin = 6;

void setup()
{
  // Setup Serial connection
  Serial.begin(115200);

  pinMode(relayPin, OUTPUT);
  //pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin, HIGH); // HIGH=Off, LOW=On
  //digitalWrite(relayPin2, LOW);
  pinMode(ledPin, OUTPUT);    //initialize the LED pin as an output
  pinMode(buttonPin, INPUT);  //initialize the pushbutton pin as an input

  // Initialize the rtc object
  rtc.begin();
  // The following line can be uncommented to set the date and time
//    setDateTime();

  logInfo(rtc.getTime());
}

void loop()
{
  // Get data from the DS3231
  timeInfo = rtc.getTime();
  
  if (!autoSwitchOn) {  // if scheduled timer is on then ignore button presses, else switch pump for 2 hours
    manualSwitching(); // This routine toggles the relay/led when the puch button is pressed for a set period of time (2 hours=7,200,000 milli sec)
  }

  if (!manualSwitchOn) {  // engage/disengage water pump
    if (timerSwitchOn(0) && (digitalRead(relayPin) == HIGH)) { // if within scheduled irrigation time and relay is off
      comments = "Scheduled switch on";
      autoSwitchOn = true;
      turnRelayOn();
    }
    if (!timerSwitchOn(0) && (digitalRead(relayPin) == LOW)) { // if outside scheduled irrigation time and relay is on
      comments  = "Scheduled switch off";
      autoSwitchOn = false;
      turnRelayOff();
    }
  }
}

void turnRelayOn() {
  if (digitalRead(relayPin) == HIGH) {
    digitalWrite(relayPin, LOW);
    digitalWrite(ledPin, HIGH);
    logInfo(timeInfo);
  }
}

void turnRelayOff() {
  if (digitalRead(relayPin) == LOW) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(ledPin, LOW);
    logInfo(timeInfo);
  }
}
void printStates() {
  Serial.print("ledState: ");
  Serial.print(ledState);

  Serial.print(" relayPin: ");
  Serial.println(digitalRead(relayPin));
}

void manualSwitching() {

  if (buttonPressed()) {
    toggle();
    if (ledState == HIGH) {
      manualSwitchOn = true;
      stopWatch = millis();
      comments = "manual timer started";
      turnRelayOn();
    }
    else {
      manualSwitchOn = false;
      comments = "manual timer stopped";
      turnRelayOff();
    }
  }

  if (manualSwitchOn && (millis() - stopWatch > irrTimer)) {
    comments = "manual timer expired";
    manualSwitchOn = false;
    turnRelayOff();
    toggle();
  }

  //digitalWrite(ledPin, ledState);
  lastButtonState = reading;
}

bool buttonPressed() {
  reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    //Serial.println("millis() - lastDebounceTime) > debounceDelay");
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        return true;
      }
    }
  }
  return false;
}


void toggle() {
  ledState = !ledState;
}

bool timerSwitchOn(int switchNo) {
  if (isWateringMonth(switchNo)) {
    if (isWateringDay(switchNo)) {
      if (isWateringHour(switchNo)) {
        if (isWateringMinute()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool isWateringMonth(int switchNo) {
  for (int i = 0; i < sizeof(wateringMonths[switchNo]) / sizeof (int); i++) {
    if (wateringMonths[switchNo][i] == timeInfo.mon) {
      return true;
    }
  }
  return false;
}
bool isWateringDay(int switchNo) {
  for (int i = 0; i < sizeof(wateringDays[switchNo]) / sizeof (int); i++) {
    if (wateringDays[switchNo][i] == timeInfo.dow) {      
      return true;
    }
  }
  return false;
}
bool isWateringHour(int switchNo) {
  for (int i = 0; i < sizeof(wateringHours[switchNo]) / sizeof (int); i++) {
    if (wateringHours[switchNo][i] == timeInfo.hour) {
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
      if (wateringMins[i] == timeInfo.min) {
        return true;
      }
    }
  }
  return false;
}
void logInfo(Time  t) {
  char buff[32];
  sprintf(buff, "%02d,%02d,%02d,%02d,%02d,%02d,%01d,%01d", t.year, t.mon, t.date, t.hour, t.min, t.sec, !digitalRead(relayPin), ledState);
  Serial.print(buff);
  Serial.print(" ");
  Serial.println(comments);
}
void setDateTime() {
  rtc.setDOW(SUNDAY);     // Set Day-of-Week to SUNDAY
  rtc.setTime(11, 34, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(31, 5, 2020);   // Set the date to January 1st, 2014
}
