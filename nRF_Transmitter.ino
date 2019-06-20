//*****************************Arduino Code for Transmitter***********************
//This sketch is from a tutorial video on the ForceTronics YouTube Channel. The tutorial discusses how to build a
//shield and a prototyping board for the nRF24L01 Transceiver Module.
//the code was leverage from Ping pair example at http://tmrh20.github.io/RF24/pingpair_ack_8ino-example.html
//This sketch is free to the public to use and modify at your own risk

#include <SPI.h> //Call SPI library so you can communicate with the nRF24L01+
#include <nRF24L01.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <RF24.h> //nRF2401 libarary found at https://github.com/tmrh20/RF24/
#include <printf.h>

const int fsp_DestTank_1    = 2;      //Pin mapping. fsp = float switch pin for switch at first water tank
const int fsp_DestTank_2    = 3;      //Pin mapping. fsp = float switch pin for switch at second water tank
const int pinCE             = 9;      //This pin is used to set the nRF24 to standby (0) or active mode (1)
const int pinCSN            = 10;     //This pin is used to tell the nRF24 whether the SPI communication is a command or message to send out

int seq = 0;
int ss_DestTank_1 = 0; //ss = switch state
int ss_DestTank_2 = 0;
unsigned int switchState = 0;
unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000; // send once per second

RF24 radio(pinCE, pinCSN); // Create your nRF24 object or wireless SPI connection
//  #define DATARATE RF24_2MBPS
//  #define DATARATE RF24_1MBPS
#define DATARATE RF24_250KBPS

const uint64_t pAddress = 0xE8E8F0F0E1LL; // Radio pipe addresses for the 2 nodes to communicate on.

void setup()
{
  pinMode(fsp_DestTank_1, INPUT_PULLUP);
  pinMode(fsp_DestTank_2, INPUT_PULLUP);
  
  Serial.begin(115200);
  printf_begin();
  //

  radio.begin();  //Start the nRF24 module
  radio.openWritingPipe(pAddress);      // pipe address that we will communicate over, must be the same for each nRF24 module

  radio.setDataRate( DATARATE ) ;
  radio.setPALevel( RF24_PA_MAX ) ;
  radio.setChannel(0x34);
  radio.enableDynamicPayloads() ;
  radio.enableAckPayload();               // not used here

  radio.setRetries(3, 5);                // Smallest time between retries, max no. of retries
  radio.setAutoAck( true ) ;
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  radio.powerUp();
  //radio.stopListening();
}

void loop()
{
  currentMillis = millis();
    if (currentMillis - prevMillis >= txIntervalMillis) {
        send();
        prevMillis = millis();
    }
}
void send()
{
  ss_DestTank_1 = digitalRead(fsp_DestTank_1);
  ss_DestTank_2 = digitalRead(fsp_DestTank_2);
  
  switchState = (ss_DestTank_1 * 0.1 + ss_DestTank_2 * 0.01) * 100;
  if (!radio.write(&switchState, sizeof(switchState) )) { //if the send fails let the user know over serial monitor
  Serial.print(F("packet delivery failed "));
  }

  Serial.println(switchState);
  delay(1000);
}
