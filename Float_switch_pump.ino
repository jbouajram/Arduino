/* This sketch is meant to control water pump activation. It makes the decision based on the input of 
 *  three float switches. the first switch is the one located at the bottom of the tank we want
 *  to pump from. this switch sits at the bottom on the from tank with switch movements facing 
 *  upward. for the pump to work this switch must always be open.
 *  the other two switched are located at the pump-to tank (on the roof). one is located at the top of the tank
 *  with switch movement facing downwards. when closed this switch will make sure the pump stops pumping. the other 
 *  one is located at the middle-to-bottom end of the to-tank and it faces downwards. it is supposed to kick off
 *  the pumping mechanisim.
 */
#include <LiquidCrystal_I2C.h> // includes the LiquidCrystal Library

LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Pin mapping. fsp = float switch pin
const int fsp_FromTank = 7;
const int fsp_ToTank_Top = 8;
const int fsp_ToTank_Bottom = 9;
const int pumpRelayPin =  3; 

//ss = switch state
int ss_FromTank = 0;
int ss_ToTank_Top = 0;
int ss_ToTank_Bottom = 0;

int prev_ss_FromTank = 0;
int prev_ss_ToTank_Top = 0;
int prev_ss_ToTank_Bottom = 0;

int switchingState = 0;
int prevSwitchingState = 0;
String displayString="";

void setup() {
  pinMode(pumpRelayPin, OUTPUT);      
  pinMode(fsp_FromTank, INPUT_PULLUP);  
  pinMode(fsp_ToTank_Top, INPUT_PULLUP);  
  pinMode(fsp_ToTank_Bottom, INPUT_PULLUP);  
  
  lcd.begin(16,2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  Serial.begin(9600);
  displayResults();
}

void loop(){
  //Read switchs state (open or closed?)
  ss_FromTank = digitalRead(fsp_FromTank);
  ss_ToTank_Top = digitalRead(fsp_ToTank_Top);
  ss_ToTank_Bottom = digitalRead(fsp_ToTank_Bottom);
  /*
   * switchingState = ss_FromTank + ss_ToTank_Top + ss_ToTank_Bottom
   * possible values:
   * 111 = pump on
   * 110 = pump on
   * 101 = pump off
   * 100 = pump off
   * 011 = pump off
   * 010 = pump off
   * 001 = pump off
   * 000 = pump off
   */
  switchingState = (ss_FromTank * 0.1 + ss_ToTank_Top * 0.01 + ss_ToTank_Bottom * 0.001) * 1000;
  
  if (switchingState != prevSwitchingState){
    prevSwitchingState = switchingState;
    prev_ss_FromTank = ss_FromTank;
    prev_ss_ToTank_Top = ss_ToTank_Top;
    prev_ss_ToTank_Bottom = ss_ToTank_Bottom;
    displayResults();
    if (switchingState >= 110){
      displayLCDSerial("Pumping");
      digitalWrite(pumpRelayPin, LOW);
    }else{
      digitalWrite(pumpRelayPin, HIGH);
  }
  }

  delay(1000);
  }

  void displayResults(){
    displayString = "";
    displayString.concat("switches[");
    displayString.concat(ss_FromTank);
    displayString.concat(ss_ToTank_Top);
    displayString.concat(ss_ToTank_Bottom);
    displayString.concat("]");

    //Informational display
    if (!ss_ToTank_Top){
       displayLCDSerial("Top tank full.");
    }
    if (ss_ToTank_Bottom){
      displayLCDSerial("Top tank empty.");
    }
    if (!ss_FromTank){
      displayLCDSerial("Lower tank empty.");
    }
    //
  }
  void displayLCDSerial(String str){
    lcd.home ();
    lcd.clear();
    lcd.print(displayString);
    Serial.println(displayString);
    lcd.setCursor(0,1);
    lcd.print(str);
    Serial.println(str);

  }
