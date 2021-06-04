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
int Relay = 4;                              // To declare the relay pin

//Button
#include <ezButton.h>                       // To include the ezButton library
ezButton peebutton(7);                      // To declare the button pin

//Waterflow sensor
int x;                                      // To declare variable x
int y;                                      // To declare variable y
float tijd = 0;                             // To declare tijd
float frequentie = 0;                       // To declare frequentie
float water = 0;                            // To declare water
const int input = A0;                       // To declare the input pin
String waterstring;                         // To declare a

//LoRaWAN
#include <MKRWAN.h>                         //
LoRaModem modem;                            //
#include "arduino_secrets.h"                //
String appEui = SECRET_APP_EUI;             //
String appKey = SECRET_APP_KEY;             //

//IPEE
int var = 0;                                //
String incoming;                            //
String clogged = "1";
String unclogged = "0";

void setup() {

  Serial.begin(9600);                       //

  //Waterflow sensor
  pinMode(input, INPUT);                    //

  //Button
  pinMode(6, INPUT_PULLUP);                 //Set Pin6 as output
  pinMode(Relay, OUTPUT);                   //
  peebutton.setDebounceTime(100);           //

  //LoRaWAN
  while (!Serial);                          //
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start");      //
    while (1) {}                            //
  };
  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong");
    while (1) {}
  }
  modem.minPollInterval(60);

  //IPEE
  Serial1.begin(9600);
  delay(5000);
  while (var < 5) {
    byte message[] = {0xFF, 0x04, 0x00, 233};
    Serial1.write(message, sizeof(message));
    var = var + 1;
    delay(500);
  }
}

void loop() {
  Button();
  NFC();
  //waterflow();
  IPEE_sensor();
}

void Button() {
  peebutton.loop();
  if (peebutton.isPressed()) {
    digitalWrite(Relay, LOW);
    delay(1000);
  }
  else {
    digitalWrite(Relay, HIGH);
  }
}
void NFC() {
  if (nfc.scan()) {
    if (nfc.readData(dataRead, READ_BLOCK_NO) != 1) {
      Serial.print("Block ");
      Serial.print(READ_BLOCK_NO);
      Serial.println(" read failure!");
    }
    else {
      Serial.print("Block ");
      Serial.print(READ_BLOCK_NO);
      Serial.println(" read success!");

      Serial.print("Data read(string):");
      Serial.println((char *)dataRead);
      Serial.print("Data read(HEX):");
      digitalWrite(Relay, LOW);
      for (int i = 0; i < BLOCK_SIZE; i++) {
        Serial.print(dataRead[i], HEX);
        Serial.print(" ");
        dataRead[i] = 0;
      }
      Serial.println();
    }
    delay(500);
  }
}

void waterflow() {
  x = pulseIn(input, HIGH);
  y = pulseIn(input, LOW);
  tijd = x + y;
  frequentie  = 1000000 / tijd;
  water = frequentie / 7.5;
  waterstring = int(water);

  if (frequentie >= 0) {
    if (isinf(frequentie)) {
      Serial.println("Volume: 0.00");
    } else {
      Serial.print("Volume: ");
      Serial.print(water);
      Serial.println(" l/m");
      String msg = waterstring;

      Serial.println();
      Serial.print("Sending: " + msg + " - ");
      for (unsigned int i = 0; i < msg.length(); i++) {
        Serial.print(msg[i] >> 4, HEX);
        Serial.print(msg[i] & 0xF, HEX);
        Serial.print(" ");
      }
      Serial.println();

      int err;
      modem.beginPacket();
      modem.print(msg);
      err = modem.endPacket(true);
      if (err > 0) {
        Serial.println("Message sent correctly!");
      } else {
        Serial.println("Error sending message :(");
      }
      delay(150000);
    }
  }
  delay(300);
}
void IPEE_sensor() {
  while (Serial1.available() > 0) {
    int incoming = Serial1.read();
    Serial.println(incoming, HEX);
    if (incoming == "FF51144") {
      digitalWrite(Relay, HIGH);
      String msg = clogged;

      Serial.println();
      Serial.print("Sending: " + msg + " - ");
      for (unsigned int i = 0; i < msg.length(); i++) {
        Serial.print(msg[i] >> 4, HEX);
        Serial.print(msg[i] & 0xF, HEX);
        Serial.print(" ");
      }
      Serial.println();

      int err;
      modem.beginPacket();
      modem.print(msg);
      err = modem.endPacket(true);
      if (err > 0) {
        Serial.println("Message sent correctly!");
      } else {
        Serial.println("Error sending message :(");
      }
      delay(150000);
    }
  if (incoming == "FF5101A") {
      Serial.println("All is fine");
      String msg = unclogged;

      Serial.println();
      Serial.print("Sending: " + msg + " - ");
      for (unsigned int i = 0; i < msg.length(); i++) {
        Serial.print(msg[i] >> 4, HEX);
        Serial.print(msg[i] & 0xF, HEX);
        Serial.print(" ");
      }
      Serial.println();

      int err;
      modem.beginPacket();
      modem.print(msg);
      err = modem.endPacket(true);
      if (err > 0) {
        Serial.println("Message sent correctly!");
      } else {
        Serial.println("Error sending message :(");
      }
      delay(150000);
    }
  }
}
