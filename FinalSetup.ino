/*!
    ICLOG

    This project is about automating an airport bathroom. When a clogging has been detected by the IPEE sensor, a door will be lockd and a message will be sent via the LoRaWAN network.
    The message will reach our database and Grafana using The Things Network, Pipedream that sends a http webhook to our Raspberry Pi and Flask that inserts the data into MySQL. Two readings
    will be sent via LoRaWAN: an analog reading from our water flow sensor and a 0 or 1 reading from our UART IPEE sensor. These readings will be displayed on our Grafana dashboard that will
    notify the cleaning staff in case of a clogging. 

    The circuit:
      Input:
        - Button on pins 7 and GND to unlock the door if a person is trapped
        - NFC reader using pins SDA, SCL, GND and 5V to unlock the door if the cleaning staff scans their badge
        - Water flow sensor using pins A0, GND and 5V to measure the use of water
        - IPEE sensor using pins RX, TX, 5V and GND to measure cloggings 
      Output:
        - Solenoid lock connected via a relay on pin 4 to lock the door in case of a clogging
        - LoRaWAN connectivity using the onboard antenna connector to send clogging and water flow readings via the LoRaWAN network

     Created on 20/05/2021
     By Senne De Winter and Tibo Smet
     Modified on 6/06/2021
     By Senne De Winter
      
*/
//NFC reader
#include <DFRobot_PN532.h>                                   // To include the DFRobot_PN532 library

#define BLOCK_SIZE       16                                  // To define the block size
#define  PN532_IRQ        2                                  // To define the interrupt request
#define  INTERRUPT        1                                  // To define the interrupt
#define  POLLING          0                                  // To define the polling

#define  READ_BLOCK_NO    2                                  // To declare the block to be read

DFRobot_PN532_IIC  nfc(PN532_IRQ, POLLING);                  // To create object nfc
uint8_t dataRead[16] = {0};                                  // To declare dataRead

//Solenoid Lock
int Relay = 4;                                               // To declare relay on pin 4

//Button
#include <ezButton.h>                                        // To include the ezButton library
ezButton peebutton(7);                                       // To declare peebutton on pin 7

//Waterflow sensor
int x;                                                       // To declare x
int y;                                                       // To declare y
float tijd = 0;                                              // To declare tijd
float frequentie = 0;                                        // To declare frequentie
float water = 0;                                             // To declare water
const int input = A0;                                        // To declare input on pin A0
String waterdata;                                            // To declare waterstring

//LoRaWAN
#include <MKRWAN.h>                                          // To include the MKRWAN library
LoRaModem modem;                                             // To set the LoRa Modem
#include "arduino_secrets.h"                                 // To include the arduino_secrets.h 
String appEui = SECRET_APP_EUI;                              // To use the APP_EUI from arduino_secrets.h
String appKey = SECRET_APP_KEY;                              // To use the APP_KEY from arduino_secrets.h

//IPEE
int var = 0;                                                 // To declare var

//Chrono
#include <Chrono.h>                                          // To include the Chrono library
Chrono waitSerial;                                           // To create timer waitSerial
Chrono sendCalibration;                                      // To create timer sendCalibration
Chrono unlockDoorNFC;                                        // To create timer unlockDoorNFC
Chrono unlockDoorButton;                                     // To create timer unlockDoorButton
Chrono sendWaterLoRa;                                        // To create timer sendWaterLoRa

void setup() {

  Serial.begin(9600);                                        // To begin serial communication at baudrate 9600

  //Waterflow sensor
  pinMode(input, INPUT);                                     // To set input pinmode for input to input

  //Button
  pinMode(6, INPUT_PULLUP);                                  // To set Pin 6 as an output
  pinMode(Relay, OUTPUT);                                    // To set relay as an output
  peebutton.setDebounceTime(100);                            // To set the debounce time for peebutton to 100ms

  //LoRaWAN
  while (!Serial);                                           // To execute when Serial is not available
  if (!modem.begin(EU868)) {                                 // To execute when modem has not started yet
    Serial.println("Failed to start");                       // To print an error message 
    while (1) {}                                             // To keep looping when previous errors are true
  };
  int connected = modem.joinOTAA(appEui, appKey);            // To join the LoRaWAN network
  if (!connected) {                                          // To execute when Arduino is not connected to the network
    Serial.println("Something went wrong");                  // To print an error message
    while (1) {}                                             // To keep looping when previous error is true
  }
  modem.minPollInterval(60);                                 // To set the modem poll interval to 60 seconds

  //IPEE
  Serial1.begin(9600);                                       // To begin Serial1 at 9600 baudrate 
  if(waitSerial.hasPassed(5000)){                            // To wait for 5 seconds
    while (var < 5) {                                        // To execute 5 times
      if(sendCalibration.hasPassed(500)){                    // To wait for 500 ms
        byte message[] = {0xFF, 0x04, 0x00, 233};            // To set the message as a byte
        Serial1.write(message, sizeof(message));             // To send the message over UART
        var = var + 1;                                       // To count to 5
        sendCalibration.restart();                           // To restart the timer
      }
    }
  }
}

void loop() {
  Button();                                                  // To call the Button function
  NFC();                                                     // To call the NFC function
  waterflow();                                               // To call the waterflow function
  IPEE_sensor();                                             // To call the IPEE_sensor function
}

void Button() {
  peebutton.loop();                                          // To start the button loop
  if (peebutton.isPressed()) {                               // To execute when peebutton is pressed
    if(unlockDoorButton.hasPassed(4000)){                    // To wait for 4 seconds
      digitalWrite(Relay, LOW);                              // To set the relay to low
      unlockDoorButton.restart();                            // To wait for a second
    }
  }
  else { 
    digitalWrite(Relay, HIGH);                               // To set the relay to high when peebutton is not pressed
  }
}
void NFC() {
  if (nfc.scan()) {                                          // To scan the NFC module
    if (nfc.readData(dataRead, READ_BLOCK_NO) != 1) {        // To execute when the reading failed
      Serial.print("Block ");                                // To print block
      Serial.print(READ_BLOCK_NO);                           // To print the block that has been read
      Serial.println(" read failure!");                      // To print read failure
    } 
    else {
      Serial.print("Block ");                                // To print block
      Serial.print(READ_BLOCK_NO);                           // To print the block that has been read
      Serial.println(" read success!");                      // To print read succes

      Serial.print("Data read(string):");                    // To print data read(string):
      Serial.println((char *)dataRead);                      // To print the read data
      Serial.print("Data read(HEX):");                       // To print data read(HEX):
      if(unlockDoorNFC.hasPassed(4000)){                     // To wait for 4 seconds
        digitalWrite(Relay, LOW);                            // To set relay to low
        unlockDoorNFC.restart();                             // To restart the timer
      }
      for (int i = 0; i < BLOCK_SIZE; i++) {                 // To print the read data ,character per character
        Serial.print(dataRead[i], HEX);                      // To print the read data in hexadecimal        
        Serial.print(" ");                                   // To print blank space
        dataRead[i] = 0;                                     // To set dataRead to 0
      }
      Serial.println();                                      // To print a blank line
    }
  }
}

void waterflow() {
  x = pulseIn(input, HIGH);                                  // To set x to HIGH pulses      
  y = pulseIn(input, LOW);                                   // To set y to LOW pulses
  tijd = x + y;                                              // To calculate time
  frequentie  = 1000000 / tijd;                              // To calculate frequency
  water = frequentie / 7.5;                                  // To calculate the water flow
  waterdata = int(water);                                    // To convert the water flow to an integer
 
  if (frequentie >= 0) {                                     // To execute if the frequency is greater than or 0
    if (isinf(frequentie)) {                                 // To execute if the frequency is infinite
      Serial.println("Volume: 0.00");                        // To print volume = 0
    } else {
      Serial.print("Volume: ");                              // To print volume when the frequency is not infinite
      Serial.print(water);                                   // To print the water flow
      Serial.println(" l/m");                                // To print liters per minute
      String msg = waterdata;                                // To set the LoRaWAN message
 
      Serial.println();                                      // To print a blank line
      Serial.print("Sending: " + msg + " - ");               // To print the message that is going to be sent
      for (unsigned int i = 0; i < msg.length(); i++) {      // To print the message
        Serial.print(msg[i] >> 4, HEX);                      // To print the message in hexadecimal
        Serial.print(msg[i] & 0xF, HEX);                     // To print the message in hexadecimal
        Serial.print(" ");                                   // To print a blank space
      }
      Serial.println();                                      // To print a blank line
  
      int err;                                               // To declare err
      if(sendWaterLoRa.hasPassed(120000)){                   // To wait for 2 minutes
        modem.beginPacket();                                 // To begin a packet
        modem.print(msg);                                    // To send the message
        err = modem.endPacket(true);                         // To end the packet
        if (err > 0) {                                       // To execute when there were no errors
          Serial.println("Message sent correctly!");         // To print that the message was sent correctly
        } else {
          Serial.println("Error sending message :(");        // To print an eroor
        }
        sendWaterLoRa.restart();                             // To restart the timer
      }
    }
  }
}
void IPEE_sensor(){
   while(Serial1.available() > 0){                          // To execute when Serial1 is available
    int incoming=Serial1.read();                            // To read the incoming message                          
    Serial.println(incoming, HEX);                          // To print the message
  
    while (!Serial.available());                            // To wait until Serial is available
    String msg = Serial.readStringUntil('\n');              // To set the message to the user input via serial monitor
  
    Serial.println();                                       // To print a blank line
      Serial.print("Sending: " + msg + " - ");              // To print the message that is going to be sent
      for (unsigned int i = 0; i < msg.length(); i++) {     // To print the message
        Serial.print(msg[i] >> 4, HEX);                     // To print the message in hexadecimal
        Serial.print(msg[i] & 0xF, HEX);                    // To print the message in hexadecimal
        Serial.print(" ");                                  // To print a blank space
      }
      Serial.println();                                     // To print a blank line
  
      int err;                                              // To declare an error
      modem.beginPacket();                                  // To begin a packet
      modem.print(msg);                                     // To send the message
      err = modem.endPacket(true);                          // To end the packet
      if (err > 0) {                                        // To execute when there were no errors
        Serial.println("Message sent correctly!");          // To print that the message was sent correctly
      } else {
        Serial.println("Error sending message :(");         // To print an error
      }
    }
}
