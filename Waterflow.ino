int x;
int y;
float tijd = 0;
float frequentie = 0;
float water = 0;

const int input = A0;

void setup() {
  Serial.begin(9600);
  pinMode(input,INPUT);
}

void loop() {
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
