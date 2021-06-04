/*!
    ICLOG
*/
//NFC reader
#include <DFRobot_PN532.h>                  // To include the DFRobot_PN532 library

#define BLOCK_SIZE       16                 // To define the block size
#define  PN532_IRQ        2                 // To define the interrupt request
#define  INTERRUPT        1                 // To define the interrupt
#define  POLLING          0                 // To define the polling

#define  READ_BLOCK_NO    2                 // To declare the block to be read

DFRobot_PN532_IIC  nfc(PN532_IRQ, POLLING); //
uint8_t dataRead[16] = {0};                 //

//Solenoid Lock
int Relay = 4;                              // To declare relay on pin 4

//Button
#include <ezButton.h>                       // To include the ezButton library
ezButton peebutton(7);                      // To declare peebutton on pin 7

//Waterflow sensor
int x;                                      // To declare x
int y;                                      // To declare y
float tijd = 0;                             // To declare tijd
float frequentie = 0;                       // To declare frequentie
float water = 0;                            // To declare water
const int input = A0;                       // To declare input on pin A0
String waterstring;                         // To declare waterstring

//LoRaWAN
#include <MKRWAN.h>                         // To include the MKRWAN library
LoRaModem modem;                            // To set the LoRa Modem
#include "arduino_secrets.h"                // To include the arduino_secrets.h 
String appEui = SECRET_APP_EUI;             // To use the APP_EUI from arduino_secrets.h
String appKey = SECRET_APP_KEY;             // To use the APP_KEY from arduino_secrets.h

//IPEE
int var = 0;                                // To declare var
String incoming;                            // To declare incoming
String clogged = "1";                       // To declare clogged
String unclogged = "0";                     // To declare unclogged

void setup() {

  Serial.begin(9600);                       // To begin serial communication at baudrate 9600

  //Waterflow sensor
  pinMode(input, INPUT);                    // To set input pinmode for input to input

  //Button
  pinMode(6, INPUT_PULLUP);                 // To set Pin 6 as an output
  pinMode(Relay, OUTPUT);                   // To set relay as an output
  peebutton.setDebounceTime(100);           // To set the debounce time for peebutton to 100ms

  //LoRaWAN
  while (!Serial);                          // To execute when Serial is not available
  if (!modem.begin(EU868)) {                // To execute when modem has not started yet
    Serial.println("Failed to start");      // To print an error message 
    while (1) {}                            // To keep looping when previous errors are true
  };
  int connected = modem.joinOTAA(appEui, appKey);      // To join the LoRaWAN network
  if (!connected) {                                    // To execute when Arduino is not connected to the network
    Serial.println("Something went wrong");            // To print an error message
    while (1) {}                                       // To keep looping when previous error is true
  }
  modem.minPollInterval(60);                // To set the modem poll interval to 60 seconds

  //IPEE
  Serial1.begin(9600);                            // To begin Serial1 at 9600 baudrate 
  delay(5000);                                    // To wait for 5 seconds
  while (var < 5) {                               // To execute 5 times
    byte message[] = {0xFF, 0x04, 0x00, 233};     // To set the message as a byte
    Serial1.write(message, sizeof(message));      // To send the message over UART
    var = var + 1;                                // To count to 5
    delay(500);                                   // To wait for halfa second
  }
}

void loop() {
  Button();                                       // To call the Button function
  NFC();                                          // To call the NFC function
  waterflow();                                    // To call the waterflow function
  IPEE_sensor();                                  // To call the IPEE_sensor function
}

void Button() {
  peebutton.loop();                               // To start the button loop
  if (peebutton.isPressed()) {                    // To execute when peebutton is pressed
    digitalWrite(Relay, LOW);                     // To set the relay to low
    delay(1000);                                  // To wait for a second
  }
  else { 
    digitalWrite(Relay, HIGH);                    // To set the relay to high when peebutton is not pressed
  }
}
void NFC() {
  if (nfc.scan()) {                                      // To scan the NFC module
    if (nfc.readData(dataRead, READ_BLOCK_NO) != 1) {    // To execute when the reading failed
      Serial.print("Block ");                            // To print block
      Serial.print(READ_BLOCK_NO);                       // To print the block that has been read
      Serial.println(" read failure!");                  // To print read failure
    } 
    else {
      Serial.print("Block ");                            // To print block
      Serial.print(READ_BLOCK_NO);                       // To print the block that has been read
      Serial.println(" read success!");                  // To print read succes

      Serial.print("Data read(string):");                // To print data read(string):
      Serial.println((char *)dataRead);                  // To print the read data
      Serial.print("Data read(HEX):");                   // To print data read(HEX):
      digitalWrite(Relay, LOW);                          // To set relay to low
      for (int i = 0; i < BLOCK_SIZE; i++) {             // To print the read data ,character per character
        Serial.print(dataRead[i], HEX);                  // To print the read data in hexadecimal        
        Serial.print(" ");                               // To print blank space
        dataRead[i] = 0;                                 // To set dataRead to 0
      }
      Serial.println();                                  // To print a blank line
    }
    delay(500);                                          // To wait for half a second
  }
}

void waterflow() {
  x = pulseIn(input, HIGH);                              // To set x to HIGH pulses      
  y = pulseIn(input, LOW);                               // To set y to LOW pulses
  tijd = x + y;                                          // To calculate time
  frequentie  = 1000000 / tijd;                          // To calculate frequency
  water = frequentie / 7.5;                              // To calculate the water flow
  waterdata = int(water);                                // To convert the water flow to an integer
 
  if (frequentie >= 0) {                                 // To execute if the frequency is greater than or 0
    if (isinf(frequentie)) {                             // To execute if the frequency is infinite
      Serial.println("Volume: 0.00");                    // To print volume = 0
    } else {
      Serial.print("Volume: ");                          // To print volume when the frequency is not infinite
      Serial.print(water);                               // To print the water flow
      Serial.println(" l/m");                            // To print liters per minute
      String msg = waterdata;                            // To set the LoRaWAN message
 
      Serial.println();                                    // To print a blank line
      Serial.print("Sending: " + msg + " - ");             // To print the message that is going to be sent
      for (unsigned int i = 0; i < msg.length(); i++) {    // To print the message
        Serial.print(msg[i] >> 4, HEX);                    // To print the message in hexadecimal
        Serial.print(msg[i] & 0xF, HEX);                   // To print the message in hexadecimal
        Serial.print(" ");                                 // To print a blank space
      }
      Serial.println();                                    // To print a blank line
  
      int err;                                             // To declare an error
      modem.beginPacket();                                 // To begin a packet
      modem.print(msg);                                    // To send the message
      err = modem.endPacket(true);                         // To end the packet
      if (err > 0) {                                       // To execute when there were no errors
        Serial.println("Message sent correctly!");         // To print that the message was sent correctly
      } else {
        Serial.println("Error sending message :(");        // To print an eroor
      }
      delay(150000);                                       // To wait for 2.5 minutes
    }
  }
}
void IPEE_sensor() {
  const char* cloggedbyte[5] = {"FF51144"};                // To declare the clogged byte array
  const char* uncloggedbyte[5] = {"FF5101A"};              // To declare the unclogged byte array
  while (Serial1.available() > 0) {                        // To execute when Serial1 is available
    int incomingbyte = Serial1.read();                     // To read the incoming byte
    const char* incoming[5] = {incomingbyte};              // To convert the incoming byte
   
    
    if (incoming == cloggedbyte) {                         // To execute when the incoming byte is clogged
      digitalWrite(Relay, HIGH);                           // To set the relay to high
      String msg = "1";                                    // To set the LoRaWAN message to 1

      Serial.println();                                    // To print a blank line
      Serial.print("Sending: " + msg + " - ");             // To print the message that is going to be sent
      for (unsigned int i = 0; i < msg.length(); i++) {    // To print the message
        Serial.print(msg[i] >> 4, HEX);                    // To print the message in hexadecimal
        Serial.print(msg[i] & 0xF, HEX);                   // To print the message in hexadecimal
        Serial.print(" ");                                 // To print a blank space
      }
      Serial.println();                                    // To print a blank line
  
      int err;                                             // To declare an error
      modem.beginPacket();                                 // To begin a packet
      modem.print(msg);                                    // To send the message
      err = modem.endPacket(true);                         // To end the packet
      if (err > 0) {                                       // To execute when there were no errors
        Serial.println("Message sent correctly!");         // To print that the message was sent correctly
      } else {
        Serial.println("Error sending message :(");        // To print an eroor
      }
      delay(150000);                                       // To wait for 2.5 minutes
    }
  if (incoming == uncloggedbyte) {                         // To execute when the incoming byte is unclogged
      Serial.println("Everything is fine");                // To print that everything is fine
      String msg = "0";                                    // To set ther message to 0

      Serial.println();                                    // To print a blank line
      Serial.print("Sending: " + msg + " - ");             // To print the message that is going to be sent
      for (unsigned int i = 0; i < msg.length(); i++) {    // To print the message
        Serial.print(msg[i] >> 4, HEX);                    // To print the message in hexadecimal
        Serial.print(msg[i] & 0xF, HEX);                   // To print the message in hexadecimal
        Serial.print(" ");                                 // To print a blank space
      }
      Serial.println();                                    // To print a blank line
  
      int err;                                             // To declare an error
      modem.beginPacket();                                 // To begin a packet
      modem.print(msg);                                    // To send the message
      err = modem.endPacket(true);                         // To end the packet
      if (err > 0) {                                       // To execute when there were no errors
        Serial.println("Message sent correctly!");         // To print that the message was sent correctly
      } else {
        Serial.println("Error sending message :(");        // To print an eroor
      }
      delay(150000);                                       // To wait for 2.5 minutes
    }
  }
}
