/******************************************************************************

  Spin Table AccelStepper Big Easy Driver DIY Shield
  Author: Damon Hamm, Sculptor
  Last Updated: 2017 June 30

  A DIY Big Easy Driver shield using the Arduino Proto Shield R3.

  Modified from the original 'Spin' turtntable sketch by Tiffany Tseng
  for controlling using the Spin iOS app:
  http://spin.media.mit.edu/build
  AccelStepper driver documentation at:
  http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html
  Brian Schmalz Big Easy Driver page:
  http://www.schmalzhaus.com/BigEasyDriver/index.html

******************************************************************************/

// Pin definitions for Easy Driver on Arduino Proto Shield R3
#define pin_dirED 2
#define pin_stepED 9
/*
  // optional Easy Driver pins to control step resolution and enabled state
  #define ED_MS1 11
  #define ED_MS2 10
  #define ED_enable 8
*/
// Other declarations
#define led_power 13
#define led_mode A2

// Motor parameter variables - tweak these as necessary
int numRotations = 15; // default = 15
float rotAmt = 2600; // Distance the stepper should turn for each increment * numRotations to make a full 360 spin. Unforunately, distance seems tied to acceleration (motor_accel).
float motor_accel = 8000; // Acceleration (>0): I chose an moderately fast acceleration that doesn't shaking whatever is on top too much.
float motor_speed = 4000; // Max speed (>0): Mine caps at around 4100. Speeds exceeding max supported by the processor may result in non-linear accelerations.

#include <AccelStepper.h>
// Basic usage 'AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN_INT, DIRECTION_PIN_INT);
// 'AccelStepper::DRIVER' sets the MotorInterfaceType to use a stepper driver board with Step and Direction pins.
// If an enable line is also needed, call setEnablePin() after construction.
AccelStepper myStepper(AccelStepper::DRIVER, pin_stepED, pin_dirED);


// Variable setup
boolean rotating = false;
boolean cancelled = false;
int currentRotation = 0;
char val;
char user_input;


// Spin mobile app dependencies
#include <SoftModem.h> // library
#include <ctype.h>
// *** LIBRARY IS HARDCODED TO USE PINS D3 AND D6
// *** D3 = FSK-OUT
// *** D6 = FSK-IN
SoftModem modem; // create Softmodem object

// Button control variables
#define btn_easy 4
int buttonState = HIGH;
int lastButtonState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 20;
boolean spinCreated = false;


void setup() {
  // Easy Driver pin setup
  pinMode(pin_stepED, OUTPUT);
  pinMode(pin_dirED, OUTPUT);

  myStepper.setMaxSpeed(motor_speed);
  myStepper.setAcceleration(motor_accel);

  /*
    pinMode(ED_MS1, OUTPUT);
    pinMode(ED_MS2, OUTPUT);
    pinMode(ED_enable, OUTPUT);
    resetEDPins(); //Set Easy Driver step, direction, microstep and set pins to defaults
  */
  // Button, switch & LED pin setup
  //  pinMode(ED_enable, OUTPUT);
  //  pinMode(btn_fwd, INPUT_PULLUP);
  //  pinMode(btn_rev, INPUT_PULLUP);
  pinMode(btn_easy, INPUT_PULLUP);
  pinMode(led_mode, OUTPUT);

  Serial.begin(9600); //Open Serial connection for debugging
  Serial.println();
  Serial.println("Press: 1 rotate forward, 2 reverse, 3 spin 360, 4 sim-photo, ");
  Serial.println("5 motor speed test, 6 motor accel test:");
  modem.begin(); // start Softmodem mobile audio for communicating to Spin app
}


//////////////////// MAIN CONTROL LOOP ////////////////////
void loop() {
  //  digitalWrite(ED_enable, LOW); //Pull enable pin low to allow motor control

  // Serial port control for debugging
  if (Serial.available()) {
    user_input = Serial.read(); //Read user input and trigger appropriate function
    if (user_input == '1') {
      rotate(rotAmt);
    } else if (user_input == '2') {
      rotate(-rotAmt);
    } else if (user_input == '3') {
      rotate(rotAmt * numRotations);
    } else if (user_input == '4') {
      currentRotation = 0;
      rotating = true;
      cancelled = false;
      rotateAndPhoto();
    } else if (user_input == '5') { // for calibrating the maximum motor speed
      currentRotation = 0;
      rotating = true;
      while (currentRotation < numRotations) {
        Serial.print("motor_speed = ");
        Serial.print(motor_speed, 2);
        Serial.print("    currentRotation = ");
        Serial.println(currentRotation);
        rotate(rotAmt * 6);
        motor_speed += 100;
        myStepper.setMaxSpeed(motor_speed);
        currentRotation++;
      }
    } else if (user_input == '6') { // for calibrating the maximum motor accel
      currentRotation = 0;
      rotating = true;
      while (currentRotation < numRotations) {
        Serial.print("motor_accel = ");
        Serial.print(motor_accel, 2);
        Serial.print("    currentRotation = ");
        Serial.println(currentRotation);
        rotate(rotAmt);
        delay(4000);
        motor_accel -= 200;
        myStepper.setAcceleration(motor_accel);
        currentRotation++;
      }
    } else {
      Serial.println("Invalid option entered.");
    }
  }
  if (rotating == false) {
    ledsOff();
  }

  //////////////////// Spin mobile app modem control ////////////////////
  if (modem.available ()) {
    int input = modem.read (); // Received signal from iphone
    Serial.println(input);

    if (input == 253  || input == 93) {
      // begin spin - flash LED
      Serial.println(" start ");
      modem.write(0xFC); // send confirmation signal to app (252)
      flashLEDsOnce();
      currentRotation = 0;
      rotating = true;
      cancelled = false;
      delay(1000);
    } else if (input == 251 || input == 91) {
      // stop arduino
      rotating = false;
      cancelled = true;
      spinCreated = false;
      currentRotation = 0;
      Serial.println(" stop - writing stop signal");
      modem.write(0xFE); // 254
    } else if (input == 234 || input == 74) {
      // setup signal received (for one-time setup when user first downloads app)
      flashLEDsOnce();
      modem.write(0xEC);
      rotate(rotAmt / 2);
      rotate(-rotAmt / 2);
    } else if (input == 255) {
      // spin has been created -> (used for enabling remote shutter button)
      spinCreated = true;
    }
  }

  rotateAndPhoto();


  //////////////////// EASY BUTTON STUFF ////////////////////
  int reading = digitalRead(btn_easy);
  if (reading != lastButtonState) {

    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH && spinCreated) {
        Serial.println("write start to app");
        modem.write(0xFC); // send confirmation signal to app
        flashLEDsOnce();
        currentRotation = 0;
        rotating = true;
        cancelled = false;
        delay(1000);
      }
    }
  }
  lastButtonState = reading;
}
//////////////////// END MAIN LOOP ////////////////////


//////////////////// Run motor driver, take photo ////////////////////
void rotateAndPhoto() {
  if (rotating && !cancelled && currentRotation < numRotations) {
    Serial.println(currentRotation + 1);
    singleRotation();
    currentRotation++;
    takePhoto();
  } else {
    rotating = false;
    cancelled = true;
  }
}

void singleRotation() {
  rotating = true;
  ledsOn();
  rotate(rotAmt);
  ledsOff();
}

void takePhoto() {
  delay(1950);   // simple delay to capture photo in app - Future revision to include more device handshake feedback (success, fail, etc).

}

// Motor Controls
void rotate(float amt) {
  int dir = (amt > 0) ? HIGH : LOW;
  digitalWrite(pin_dirED, dir);

  myStepper.move(amt);
  myStepper.runToPosition();
}

//////////////////// System state feedback ////////////////////
// Future revision to include more expressive feedback (speed, direction, etc)
void ledsOn() {
  digitalWrite(led_mode, HIGH);
}

void ledsOff() {
  digitalWrite(led_mode, LOW);
}

void flashLEDs() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(led_mode, HIGH);
    delay(300);
    digitalWrite(led_mode, LOW);
    delay(300);
  }
}

void flashLEDsOnce() {
  digitalWrite(led_mode, HIGH);
  delay(100);
  digitalWrite(led_mode, LOW);
}


/*
  //Reset Easy Driver pins to default states
  void resetEDPins()
  {
  digitalWrite(pin_stepED, LOW);
  digitalWrite(pin_dirED, LOW);
  digitalWrite(ED_MS1, LOW);
  digitalWrite(ED_MS2, LOW);
  digitalWrite(ED_enable, HIGH);
  }
*/



