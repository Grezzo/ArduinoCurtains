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

const int fullsteps = 1063; // Number of steps for full open/close
const int stepperSpeed = 350;

const unsigned long dayInMillis = 86400000; // 24 * 60 * 60 * 1000

boolean alarmSet = false; // Starts off with no alarm set
unsigned long openModulo;
unsigned long closeModulo;

void setup()
{  
  AFMS.begin(); // Start the shield
  setPosition(fullsteps); // Set position to closed
  Serial.begin(9600); // set up Serial library at 9600 bps
  Serial.setTimeout(10); // Set the serial timeout to 1/100th of the standard
}

void loop()
{

  if (alarmSet) { // Check if alarm is set
    unsigned long millisModulo = millis() % 86400000; // Get time past time switched on, discard any days
    if (millisModulo == openModulo) { // Check if time since switched on is open alarm time
      Serial.println("Open Alarm!");

      moveToPosition(0); // Open

    } 
    else if (millisModulo == closeModulo) { // Check if time since switched on is close alarm time
      Serial.println("Close Alarm!");

      moveToPosition(fullsteps); //Close

    }
  }

  while (Serial.available() > 0) { // Check if there is data to read from the serial input
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

    else if (function ==1) { // Set the alarm

      unsigned long nowMillisSinceMidnight = millisSinceMidnight(Serial.parseInt(), Serial.parseInt()); // Get the time now in milliseconds since midnight
      unsigned long openMillisSinceMidnight = millisSinceMidnight(Serial.parseInt(), Serial.parseInt()); // Get the open alarm time in milliseconds since midnight
      unsigned long closeMillisSinceMidnight = millisSinceMidnight(Serial.parseInt(), Serial.parseInt()); // Get the close alarm time in milliseconds since midnight

      Serial.print("Time now in ms is: ");
      Serial.println(nowMillisSinceMidnight);
      Serial.print("Open time in ms is: ");
      Serial.println(openMillisSinceMidnight);
      Serial.print("Close time in ms is: ");
      Serial.println(closeMillisSinceMidnight);

      unsigned long openFromNowMillis = millisFromUntil(nowMillisSinceMidnight, openMillisSinceMidnight); // Get the open alarm time in milliseconds from now
      unsigned long closeFromNowMillis = millisFromUntil(nowMillisSinceMidnight, closeMillisSinceMidnight); // Get the close alarm time in milliseconds from now

      Serial.print("Open time in from now ms is: ");
      Serial.println(openFromNowMillis);
      Serial.print("Close time in from now ms is: ");
      Serial.println(closeFromNowMillis);

      unsigned long now = millis(); // Get the time since power cycle in milliseconds

      openModulo = moduloMillis(openFromNowMillis, now); // Get the open alarm time since power cycle, discard any days
      closeModulo = moduloMillis(closeFromNowMillis, now); // Get the close alarm time since power cycle, discard any days

      alarmSet = true; // Turn the alarm on
    }

    else if (function == 2) { // Set open or closed position
      int newPosition = Serial.parseInt();
      setPosition(newPosition);
    }
  }
  //  logPosition();
  stepper.runSpeedToPosition(); // Move stepper (if necesarry)
}

void moveToPosition(int goal) {
  stepper.moveTo(goal);
  if (goal < stepper.currentPosition()) { // Set direction to move
    stepper.setSpeed(-stepperSpeed);
  } 
  else {
    stepper.setSpeed(stepperSpeed);
  }  
}

void setPosition(int newPosition) {
  stepper.setCurrentPosition(newPosition); // Set position
  stepper.moveTo(newPosition); // Set goal position
}

unsigned long millisSinceMidnight(int hours, int mins) { //Get time in milliseconds since midnight
  return ((unsigned long)hours * 60 + mins) * 60 * 1000;
}

unsigned long millisFromUntil(unsigned long from, unsigned long until) { // Get milliseconds from one time until another (second time can be earlier, so would be next day)
  long difference = until - from;
  if (difference < 0) { // Time is next day
    return difference + dayInMillis;
  } 
  else { // Time is later today
    return difference;
  }
}

unsigned long moduloMillis(unsigned long millisFromNow, unsigned long now) { // Get milliseconds for a time since power cycle, discard any days
  return (millisFromNow + now) % dayInMillis;
}

//boolean printed = false;
//void logPosition() {
//  if (millis() % 1000 == 0) {
//    if (printed == false) {
//      Serial.print("Target Position: ");
//      Serial.println(stepper.targetPosition());
//      Serial.print("Current Position: ");
//      Serial.println(stepper.currentPosition());
//      Serial.print("Distance To Go: ");
//      Serial.println(stepper.distanceToGo());
//
//      printed = true;
//    }
//  } 
//  else {
//    printed = false;
//  } 
//}
