#define IR_RECEIVE_PIN 11
#include <IRremote.h>

enum RobotState {
  STATE_IDLE,  // Robot is idle
  STATE_STOP,  // Robot is on but stopped
  STATE_FWD,   // Robot moving forward
  STATE_BWD,   // Robot moving backward
  STATE_LEFT,  // Robot turning left
  STATE_RIGHT  // Robot turning right
};

// Pin Definitions
#define TRIG 12     // Ultrasonic Sensor Trig Pin
#define ECHO 13       // Ultrasonic Sensor Echo Pin
#define L_EN 6        // Left Motor Enable (PWM)
#define L_IN1 7       // Left Motor IN1
#define L_IN2 8       // Left Motor IN2
#define R_EN 3        // Right Motor Enable (PWM)
#define R_IN1 2       // Right Motor IN1
#define R_IN2 4       // Right Motor IN2

#define FWD 0xE718FF00       // Forward button
#define BWD 0xAD52FF00       // Backward button
#define LEFT 0xF708FF00      // Left button
#define RIGHT 0xA55AFF00     // Right button
#define STOP 0xBF40FF00      // Stop button
#define OFF 0xBA45FF00       // Off button (toggle robot on/off)

RobotState currentState = STATE_IDLE;
bool isRobotOn = false; // Tracks whether the robot is on or off

// Function Prototype
long get_dist();

void setup() {
  Serial.begin(9600);

  // Initialize IR Receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Motor Pins as Outputs
  pinMode(L_EN, OUTPUT);
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);

  // Ultrasonic Sensor Pins
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // Initialize Motors
  currentState = STATE_IDLE;
}

void loop() {
  // Check for IR commands
  if (IrReceiver.decode()) {
    uint32_t command = IrReceiver.decodedIRData.decodedRawData;

    switch (command) {
      case OFF:
        isRobotOn = !isRobotOn; // Toggle the robot's on/off state
        currentState = isRobotOn ? STATE_STOP : STATE_IDLE;
        Serial.println(isRobotOn ? "Robot ON" : "Robot OFF");
        break;

      case FWD:
        if (isRobotOn) currentState = STATE_FWD;
        break;

      case BWD:
        if (isRobotOn) currentState = STATE_BWD;
        break;

      case LEFT:
        if (isRobotOn) currentState = STATE_LEFT;
        break;

      case RIGHT:
        if (isRobotOn) currentState = STATE_RIGHT;
        break;

      case STOP:
        if (isRobotOn) currentState = STATE_STOP;
        break;
    }
    IrReceiver.resume(); // Ready for the next IR signal
  }

  // Handle robot states
  if (isRobotOn) {
    // Ultrasonic sensor to adjust speed dynamically
    long dist = get_dist();
    int speed = (dist > 0 && dist < 20) ? 255 : 150; // Increase speed if obstacle detected within 20 cm

    switch (currentState) {
      case STATE_IDLE:
        // Robot should not perform any operations in IDLE state
        digitalWrite(L_IN1, LOW);
        digitalWrite(L_IN2, LOW);
        digitalWrite(R_IN1, LOW);
        digitalWrite(R_IN2, LOW);
        analogWrite(L_EN, 0);
        analogWrite(R_EN, 0);
        break;

      case STATE_STOP:
        digitalWrite(L_IN1, LOW);
        digitalWrite(L_IN2, LOW);
        digitalWrite(R_IN1, LOW);
        digitalWrite(R_IN2, LOW);
        analogWrite(L_EN, 0);
        analogWrite(R_EN, 0);
        break;

      case STATE_FWD:
        digitalWrite(L_IN1, HIGH);
        digitalWrite(L_IN2, LOW);
        digitalWrite(R_IN1, HIGH);
        digitalWrite(R_IN2, LOW);
        analogWrite(L_EN, speed);
        analogWrite(R_EN, speed);
        break;

      case STATE_BWD:
        digitalWrite(L_IN1, LOW);
        digitalWrite(L_IN2, HIGH);
        digitalWrite(R_IN1, LOW);
        digitalWrite(R_IN2, HIGH);
        analogWrite(L_EN, speed);
        analogWrite(R_EN, speed);
        break;

      case STATE_LEFT:
        digitalWrite(L_IN1, LOW);
        digitalWrite(L_IN2, HIGH);
        digitalWrite(R_IN1, HIGH);
        digitalWrite(R_IN2, LOW);
        analogWrite(L_EN, speed / 2); // Slower turn
        analogWrite(R_EN, speed);
        break;

      case STATE_RIGHT:
        digitalWrite(L_IN1, HIGH);
        digitalWrite(L_IN2, LOW);
        digitalWrite(R_IN1, LOW);
        digitalWrite(R_IN2, HIGH);
        analogWrite(L_EN, speed);
        analogWrite(R_EN, speed / 2); // Slower turn
        break;
    }
  } else {
    // Robot is off: Set motors to idle
    digitalWrite(L_IN1, LOW);
    digitalWrite(L_IN2, LOW);
    digitalWrite(R_IN1, LOW);
    digitalWrite(R_IN2, LOW);
    analogWrite(L_EN, 0);
    analogWrite(R_EN, 0);
    currentState = STATE_IDLE;
  }
}

// Ultrasonic Sensor Function
long get_dist() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH);
  long distance = duration * 0.034 / 2; // Convert to cm
  return distance;
}
