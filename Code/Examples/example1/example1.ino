/*
Author: janek-2115
Date: 19.05.2025
Description: Simple code to showcase the use of the W83C42 driver library.
*/


#include "W83C42.h"

W83C42 Controller = W83C42(12, 11, 10, 9, 8, 7, 6, 5, 14, 13, 4, 15, 3, 2);
char buffer[32] = {0};

void setup() {
  Serial.begin(9600);
  if(Controller.begin()){               // Performs a reset routine and a self check.
    Controller.enable();                // Enables the keyboard interface, the OBF INT and attatches the interrupt to the pin supplied in the constructor
  }
}

void loop() {
  delay(2);                             // This delay is needed 
  int size = Controller.readBuffer(buffer, 32);
  for(int i = 0; i < size; i++){
    Serial.print(buffer[i]);
  }
}
