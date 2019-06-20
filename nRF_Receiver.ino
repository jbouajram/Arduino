
//This sketch is from a tutorial video on the ForceTronics YouTube Channel. The tutorial discusses how to build a
//shield and a prototyping board for the nRF24L01 Transceiver Module.
//the code was leverage from Ping pair example at http://tmrh20.github.io/RF24/pingpair_ack_8ino-example.html
//This sketch is free to the public to use and modify at your own risk

#include <SPI.h>                //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h>           //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h>               //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <LiquidCrystal_I2C.h>  // includes the LiquidCrystal Library
#include <printf.h>

LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

const int pinCE = 9;            //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN = 10;          //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out
byte bVal;                      //used to store ADC value payload from transmit module, the ADC value will be < 256 so it will fit in a byte

RF24 radio(pinCE, pinCSN); // Declare object from nRF24 library (Create your wireless SPI)
//  #define DATARATE RF24_2MBPS
//  #define DATARATE RF24_1MBPS
#define DATARATE RF24_250KBPS

const uint64_t pAddress = 0xE8E8F0F0E1LL;  //Create a pipe addresses for the 2 nodes to communicate over, the "LL" is for LongLong type
const int ledPin = 2;
const int pumpRelayPin = 3;
const int fsp_SourceTank = 4; // fsp = float switch pin
//  PIN 5 not in use
const int buzzer = 6;
const int buttonPin = 7;

unsigned int switchState = 0;
unsigned int ss_SourceTank = 0;
unsigned int audible = 1;
unsigned int readFailCounter = 0;
String lcdLine1 = "                ";
String lcdLine2 = "                ";
bool newData = false;

void(* resetFunc) (void) = 0; // builtin function to reset Arduino automatically needs to be declared at address 0.

void setup()
{
  Serial.begin(115200);
  printf_begin();
  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display

  pinMode(ledPin, OUTPUT);
  pinMode(fsp_SourceTank, INPUT_PULLUP);
  digitalWrite(ledPin, LOW);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  radio.begin();  //Start the nRF24 module
  radio.openReadingPipe(1, pAddress);     //open pipe o for recieving meassages with pipe address
  radio.setDataRate( DATARATE ) ;
  radio.setPALevel( RF24_PA_MAX ) ;
  radio.setChannel(0x34);
  radio.enableDynamicPayloads() ;
  radio.enableAckPayload();               // not used here
  radio.setRetries(0, 5);                // Smallest time between retries, max no. of retries
  radio.setAutoAck( true ) ;
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  radio.powerUp();
  radio.startListening();                 // Start listening for messages
}

void loop()
{
  processPushButton();
  ss_SourceTank = digitalRead(fsp_SourceTank);
  //switchState = 4;                      // reset awitchState to UNKNOWN value before each radio attempt
  getData();
  showData();
  processSwitchResult();
}

void getData() {
  if ( radio.available() ) {
    radio.read( &switchState, sizeof(unsigned int) ); //read the switch(s) values sent from transmitter
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    readFailCounter = 0;
    newData = true;
  }
  else {
    //Serial.println("No Radio");
    if (readFailCounter++ > 120) resetFunc(); // reset arduino after 120 consecutive radio fails ~ 2 minutes
    delay(1000);
  }
}

void showData() {
  if (newData == true) {
    Serial.print("Switch value is ");
    Serial.println(switchState);
    newData = false;
  }
}

//This function takes an Arduino analog pin reading and converts it to a voltage value
float calculateArduinoVolt(int val) {
  float volt = (float)val * (5.0 / 1023.0); //convert ADC value to voltage
  return volt;
}

void processPushButton() {
  //when button is pressed trun lcd on/off
  if (digitalRead(buttonPin) == HIGH) {
    lcd.on();
    audible = 1;
  }
  else {
    lcd.off();
    audible = 0;
  }
}

void processSwitchResult() {
  lcdLine1 = "Water available";
  // engage/disengage water pump
  if (ss_SourceTank == 1) {
    if (switchState == 1 || switchState == 10 || switchState == 11) {
      digitalWrite(pumpRelayPin, LOW);
      lcdLine1 = "Pumping...";
    }
    else {
      digitalWrite(pumpRelayPin, HIGH);
      lcdLine1 = "no pumping needd";
    }
  }
  else {
    digitalWrite(pumpRelayPin, HIGH);
    lcdLine1 = "Main Tank empty  ";
    if (audible) {
      tone(buzzer, 1000);
      delay(50);
      noTone(buzzer);
    }
  }

  if (readFailCounter > 10){
    Serial.println("No Radio");
    lcdLine1 = "Sytem on hold";
    lcdLine2 = "No signal/sensor"; // display warning/error after 10 consecutive radio fails
    digitalWrite(pumpRelayPin, HIGH);
  }
  else {
    switch (switchState) {
      case 0:
        lcdLine2 = "both Tanks full";
        break;
      case 11:
        lcdLine2 = "both Tanks low  ";
        break;
      case 10:
        lcdLine2 = "tank 1 low";
        break;
      case 1:
        lcdLine2 = "tank 2 low";
        break;
      default:
        lcdLine2 = "check sensors   ";
        break;
    }
  }

  //display results on lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1);
  lcd.setCursor(0, 1);
  lcd.print(lcdLine2);
}
