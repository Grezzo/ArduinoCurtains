#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS(0x60); // Default address, no jumpers
Adafruit_StepperMotor *myStepper = AFMS.getStepper(200, 2); // Connect stepper with 200 steps per revolution (1.8 degree) on port 2

// wrapper for the motor
void forwardstep() {  
  myStepper->onestep(FORWARD, DOUBLE);
}
void backwardstep() {  
  myStepper->onestep(BACKWARD, DOUBLE);
}
// Wrap the stepper in an AccelStepper object
AccelStepper stepper(forwardstep, backwardstep);

int fullsteps = 1063; // Number of steps for full open/close


void setup()
{  
  AFMS.begin(); // Start the shield
  setPosition(fullsteps); // Set position to closed
  Serial.begin(9600); // set up Serial library at 9600 bps
  Serial.setTimeout(10); // Set the serial timeout to 1/100th of the standard
}

void loop()
{

  while (Serial.available() > 0) { // Check if there is data to read from the serial input
    //    int goal = Serial.parseInt(); // Get the first number (function number)
    //    stepper.moveTo(goal);
    //    if (goal < stepper.currentPosition()) { // Set direction to move
    //      stepper.setSpeed(-350);
    //    } 
    //    else {
    //      stepper.setSpeed(350);
    //    }    
    int function = Serial.parseInt(); // Get the first number (function number)
    if (function == 0) { // Set curtain position
      int percentage = Serial.parseInt(); //Get desired curtain position in percent
      if (percentage > 100) { //Make sure number is between 0 and 100, if not sanitise it
        percentage = 100;
      } 
      else if (percentage < 0) {
        percentage = 0;
      }
      int goal = fullsteps * (percentage / 100.0); // Figure out how many steps from open that percentage is
      moveToPosition(goal);  
    } 
    else if (function == 2) { // Set open or closed position
      int newPosition = Serial.parseInt();
      setPosition(newPosition);
    }
  }

        logPosition();

  stepper.runSpeedToPosition(); // Move stepper (if necesarry)

}

void moveToPosition(int goal) {
  stepper.moveTo(goal);
  if (goal < stepper.currentPosition()) { // Set direction to move
    stepper.setSpeed(-350);
  } 
  else {
    stepper.setSpeed(350);
  }  
}

void setPosition(int newPosition) {
  stepper.setCurrentPosition(newPosition); // Set position
  stepper.moveTo(newPosition); // Set goal position
}

boolean printed = false;
void logPosition() {
  if (millis() % 1000 == 0) {
    if (printed == false) {
      Serial.print("Target Position: ");
      Serial.println(stepper.targetPosition());
      Serial.print("Current Position: ");
      Serial.println(stepper.currentPosition());
      Serial.print("Distance To Go: ");
      Serial.println(stepper.distanceToGo());

      printed = true;
    }
  } 
  else {
    printed = false;
  } 
}







