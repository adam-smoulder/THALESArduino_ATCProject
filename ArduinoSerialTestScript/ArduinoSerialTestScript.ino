byte byteRead;
String stringRead;

void setup() {                
// Turn the Serial Protocol ON
  Serial.begin(9600);
}

void loop() {
   /*  check if data has been sent from the computer: */
  if (Serial.available()) {
      stringRead = Serial.readStringUntil('\n');
      Serial.print("Got em!:\t"); Serial.print(stringRead); Serial.print('\n');
    }
 }
  
