#include <Servo.h>

const int forcePin = A0;
const int redPin = 11; // Pins to control multicolor LED
const int greenPin = 10;
const int bluePin = 9;
const int servoPin = 8;

const int OpenState = 0; // Box is open and checking for close conditions
const int LockTransitionState = 1; // Box is locking
const int LockedState = 2; // Box is locked and checking for opening conditions
const int OpenTransitionState = 3; // Box is opening

Servo lockServo;

int state = OpenState;
char datatype;
int data;
String stringRead;
String curNum;
double accelData[100] = {0};
int recentAccel = 0;
double minAccel;
double maxAccel;
double magData[100] = {0};
int accelInd = 0;
int magInd = 0;
int curPressure = 0;
int curAccel = 0;
bool accelCheck = false;
bool magCheck = false;
bool pressureCheck = false;


void setup() {                
// Turn the Serial Protocol ON
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  lockServo.attach(servoPin);

  analogWrite(redPin, 255);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void loop() {
  if (state == OpenState) {
    // Retrieve any magnetometer / accelerometer data from feather
    if(Serial.available()) {
      stringRead = Serial.readStringUntil('\n'); //ex: "A 100.01\n"
      datatype = stringRead.charAt(0);
      stringRead.remove(0,2);
      data = stringRead.toInt(); // Converts last char of string to int
      
      if (datatype == 'A') {
        accelData[accelInd] = data; 
        if (accelInd == 99) {accelInd = 0;}
        else {accelInd++;}
      }
      else if (datatype == 'M') {
        magData[magInd] = data;
        if (magInd == 99) {magInd = 0;}
        else {magInd++;}
      }

      // LED red
      analogWrite(greenPin, 0);
    
      // Check pressure sensor
      curPressure = analogRead(forcePin);
      if (curPressure > 0) {pressureCheck = true;}
      else {pressureCheck = false;}
    
      // Check magnetometer
      if (magData[magInd] > 500) {magCheck = true;}
      else {magCheck = false;}
    
      // Check accelerometer
      recentAccel = accelInd;
      maxAccel = accelData[recentAccel];
      minAccel = accelData[recentAccel];
      for (int i = 1; i < 10; i++) {
        if (recentAccel == -1) {recentAccel = 99;}
        curAccel = accelData[recentAccel];
        if (curAccel > maxAccel) {
          maxAccel = curAccel;
        }
        else if (curAccel < minAccel) {
          minAccel = curAccel;
        }
        recentAccel = recentAccel - 1;
      }
      if ((maxAccel - minAccel) > 100) {accelCheck = false;}
      else {accelCheck = true;}
    
      // Determine whether phone is in box
      if (pressureCheck && magCheck && accelCheck) {
        state = LockTransitionState;  
      }
    }
  }
  
  if (state == LockTransitionState) {  
    // LED yellow
    analogWrite(greenPin, 255);
    
    // Servo code to lock box

    Serial.print("L\n"); // Tell computer that it's locked

    state = LockedState;  
  }

  if (state == LockedState) {
    // LED green
    analogWrite(redPin, 0);
    
    // Check Serial for computer saying train has stopped
    if (Serial.available()) {
      stringRead = Serial.readStringUntil('\n');
      if (stringRead == "Stopped") {
        state = OpenTransitionState;
      }
    }    
  }

  if (state == OpenTransitionState) {
    //LED yellow
    analogWrite(redPin, 255);
    
    // Servo code to unlock box

    state = OpenState;
    Serial.print("U\n"); //sending Unlocked state to Feather M0
    //delay(7200); //delay on unlock (backup)
    stringRead = "";
    
    while(!stringRead.equals("Reset")) { //wait until Reset
      stringRead = Serial.readStringUntil('\n');
    }
  }

  Serial.print(""+(int)magCheck+""+(int)accelCheck+""+(int)pressureCheck+"\n"); //ex: "010\n"
}
