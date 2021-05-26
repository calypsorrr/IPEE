#include <ezButton.h>

ezButton button(7);  
void setup() {
  Serial.begin(9600);
  button.setDebounceTime(100); 
}

void loop() {
  button.loop(); 

  int btnState = button.getState();
  Serial.println(btnState);

  if(button.isPressed())
    Serial.println("The button is pressed");

  if(button.isReleased())
    Serial.println("The button is released");
}
