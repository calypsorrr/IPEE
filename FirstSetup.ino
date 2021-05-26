#include <DFRobot_PN532.h>

#define BLOCK_SIZE       16
#define  PN532_IRQ        2
#define  INTERRUPT        1
#define  POLLING          0
// The block to be read
#define  READ_BLOCK_NO    2

DFRobot_PN532_IIC  nfc(PN532_IRQ, POLLING);
uint8_t dataRead[16] = {0};

int Relay = 4;  

#include <ezButton.h>

ezButton peebutton(7);

int x;
int y;
float tijd = 0;
float frequentie = 0;
float water = 0;

const int input = A0;

void setup() {
  Serial.begin(9600);
  pinMode(input,INPUT);

  Serial.begin(115200);
  Serial.print("Initializing");
  while (!nfc.begin()) {
    Serial.print(".");
    delay (1000);
  }
  Serial.println();
  Serial.println("Waiting for a card......");
     
  pinMode(6, INPUT_PULLUP);         //Set Pin6 as output    
  pinMode(Relay, OUTPUT);

  peebutton.setDebounceTime(100);
} 

void loop() {
    Button();
    NFC();
    waterflow();
}

void Button(){
  peebutton.loop();
  if(peebutton.isPressed()){
    digitalWrite(Relay, LOW);
    delay(1000);
  }
  else{
    digitalWrite(Relay, HIGH);
  }
}
void NFC(){
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
  frequentie  = 1000000/tijd;
  water = frequentie /7.5;

  if(frequentie >= 0) {
    if(isinf(frequentie)) {
      Serial.println("Volume: 0.00");
    } else {
      Serial.print("Volume: ");
      Serial.print(water);
      Serial.println(" l/m");
    }
  delay(300);  
  }

}
