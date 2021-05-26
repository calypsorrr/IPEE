int Relay = 4;                      //To initialise the relay on pin 4

void setup() {
  Serial.begin(57600);   
  pinMode(6, INPUT_PULLUP);         //Set Pin6 as output    
  pinMode(Relay, OUTPUT);           //Set Relay as output
} 
void loop() {
  int singal=digitalRead(6);
  Serial.println(singal);           //To print the singal      
  digitalWrite(Relay, HIGH);        //To turn on the relay
  delay(2000);                      //To wait for 2 secondq
  digitalWrite(Relay, LOW);         //To turn off the relay
  delay(2000);                      //To wait for 2 seconds

}
