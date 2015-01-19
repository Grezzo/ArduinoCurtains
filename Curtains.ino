#include <Wire.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); // Create the motor shield object with the default I2C address
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2); // Connect a stepper motor with 200 steps per revolution to motor port #2 (M3 and M4)

int fullsteps = 1063; // Number of steps for full open/close
int positioninsteps = fullsteps; // Starts off assuming fully closed position

unsigned long dayInMillis = 86400000; // 24 * 60 * 60 * 1000

boolean alarmSet = false; // Starts off with no alarm set
unsigned long openModulo;
unsigned long closeModulo;

void setup() {
  Serial.begin(9600); // set up Serial library at 9600 bps
  Serial.setTimeout(10); // Set the serial timeout to 1/100th of the standard
  AFMS.begin();  // create with the default frequency 1.6KHz
  myMotor->setSpeed(150);  // rpm   
}

// Main loop checks time for alarm open/close and checks for serial input.
// Valid input is:
//   0,x where x is a integer percentage of how closed the curtains should be
//   0,hh:mm,hh:mm,hh:mm where the first time is now, the second time is open and the third time is close

void loop() {
  
  if (alarmSet) { // Check if alarm is set
    unsigned long millisModulo = millis() % 86400000; // Get time past time switched on, discard any days
    if (millisModulo == openModulo) { // Check if time since switched on is open alarm time
      Serial.println("Open Alarm!");
      myMotor->setSpeed(60);  // rpm 
      moveToPosition(0); // Open
      myMotor->setSpeed(150);  // rpm 
    } else if (millisModulo == closeModulo) { // Check if time since switched on is close alarm time
      Serial.println("Close Alarm!");
      myMotor->setSpeed(60);  // rpm 
      moveToPosition(fullsteps); //Close
      myMotor->setSpeed(150);  // rpm 
    }
  }
    
  while (Serial.available() > 0) { // Check if there is data to read from the serial input
    int function = Serial.parseInt(); // Get the first number (function number)
    if (function == 0) { // Set curtain position

      int percentage = Serial.parseInt(); //Get desired curtain position in percent
      if (percentage > 100) { //Make sure number is between 0 and 100, if not sanitise it
        percentage = 100;
      } else if (percentage < 0) {
        percentage = 0;
      }

      int percentageinsteps = fullsteps * (percentage / 100.0); // Figure out how many steps from open that percentage is
      moveToPosition(percentageinsteps); // Move the curtains to that position

    } else if (function ==1) { // Set the alarm
      
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
    
    } else if (function == 2) { // Set open or closed position
      if (Serial.parseInt() == 0) {
        positioninsteps = 0;
        Serial.println("Resetting position to open");
      } else {
        positioninsteps = fullsteps;
        Serial.println("Resetting position to closed");
      }
    }
  }
}

void moveToPosition(int positionToMoveTo) {
  int directiontomove; // Figure out the direction to move
  if (positionToMoveTo > positioninsteps) {
    directiontomove = FORWARD; // Close curatins
  } else {
    directiontomove = BACKWARD; // Open curtains
  }
  int steps = abs(positioninsteps - positionToMoveTo); // Steps to move

  Serial.println();
  Serial.print("Current position: ");
  Serial.println(positioninsteps);

  Serial.print("User wants to move to: ");
  Serial.println(positionToMoveTo);

  Serial.print("Steps to move: ");
  Serial.println(steps);

  Serial.print("Direction to move: ");
  Serial.println((directiontomove == 1) ? "Close" : "Open");

  myMotor->step(steps, directiontomove, DOUBLE); // Move the curtains
  myMotor->release(); // Release the curtains
  positioninsteps = positionToMoveTo; // Save new position
}

unsigned long millisSinceMidnight(int hours, int mins) { //Get time in milliseconds since midnight
    return ((unsigned long)hours * 60 + mins) * 60 * 1000;
}

unsigned long millisFromUntil(unsigned long from, unsigned long until) { // Get milliseconds from one time until another (second time can be earlier, so would be next day)
  long difference = until - from;
  if (difference < 0) { // Time is next day
    return difference + dayInMillis;
  } else { // Time is later today
    return difference;
  }
}

unsigned long moduloMillis(unsigned long millisFromNow, unsigned long now) { // Get milliseconds for a time since power cycle, discard any days
  return (millisFromNow + now) % dayInMillis;
}
