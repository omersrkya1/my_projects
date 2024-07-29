#include <Servo.h>

// Servo motor configuration
Servo servo1, servo2, servo3, servo4;
const int servoPins[4] = {9, 10, 3,11};  // Servo motor pins

// Buzzer configuration
const int buzzerPin = 6;

// Joystick and button configuration
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickButtonPin = 7;
const int recordButtonPin = 4;
const int playbackButtonPin = 5;
const int threshold = 100;  // Center tolerance range

// Initial servo positions
int servoPos[4] = {90, 90, 90, 90};
int positions[100][4];  // To store recorded positions
int posIndex = 0;  // Index for positions array

// Initial state
int currentState = 1;

// Buzzer melody function
void playBuzzerMelody() {
  int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};  // Do, Re, Mi, Fa, Sol, La, Si, Do
  int duration = 100;  // Duration for each note
  for (int i = 0; i < 8; i++) {
    tone(buzzerPin, melody[i], duration);
    delay(duration);
  }
  noTone(buzzerPin);
}

// Buzzer beep function
void buzzerBeep() {
  tone(buzzerPin, 1000, 200);  // Beep frequency and duration
  delay(200);
  noTone(buzzerPin);
}

// Servo position function
void setServoPosition(Servo &servo, int angle) {
  servo.write(angle);
}

// Interpolate positions for smooth transitions
void interpolatePositions(int startPos[], int endPos[], int steps, int interpolatedPositions[][4]) {
  for (int i = 0; i < steps; i++) {
    for (int j = 0; j < 4; j++) {
      interpolatedPositions[i][j] = startPos[j] + (endPos[j] - startPos[j]) * i / steps;
    }
  }
}

// Playback recorded positions
void playbackRecordedPositions() {
  playBuzzerMelody();
  for (int i = 0; i < posIndex - 1; i++) {
    int startPos[4];
    int endPos[4];
    int interpolatedPositions[120][4];
    for (int j = 0; j < 4; j++) {
      startPos[j] = positions[i][j];
      endPos[j] = positions[i + 1][j];
    }
    interpolatePositions(startPos, endPos, 120, interpolatedPositions);
    for (int j = 0; j < 120; j++) {
      for (int k = 0; k < 4; k++) {
        setServoPosition(servo1, interpolatedPositions[j][0]);
        setServoPosition(servo2, interpolatedPositions[j][1]);
        setServoPosition(servo3, interpolatedPositions[j][2]);
        setServoPosition(servo4, interpolatedPositions[j][3]);
      }
      delay(10);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Attach servos
  servo1.attach(servoPins[0]);
  servo2.attach(servoPins[1]);
  servo3.attach(servoPins[2]);
  servo4.attach(servoPins[3]);

  // Set initial servo positions
  for (int i = 0; i < 4; i++) {
    setServoPosition(servo1, servoPos[0]);
    setServoPosition(servo2, servoPos[1]);
    setServoPosition(servo3, servoPos[2]);
    setServoPosition(servo4, servoPos[3]);
  }

  pinMode(joystickButtonPin, INPUT_PULLUP);
  pinMode(recordButtonPin, INPUT_PULLUP);
  pinMode(playbackButtonPin, INPUT_PULLUP);

  // Initial state
  currentState = 1;
  Serial.println("Current State: " + String(currentState));
}

void loop() {
  int xVal = analogRead(joystickXPin);
  int yVal = analogRead(joystickYPin);

  // Check joystick button state
  if (digitalRead(joystickButtonPin) == LOW) {
    currentState = (currentState == 1) ? 2 : 1;
    Serial.println("Button Pressed: Mode Change -> State: " + String(currentState));
    delay(300);  // Debounce delay
  }

  // Check record button state
  if (digitalRead(recordButtonPin) == LOW) {
    for (int i = 0; i < 4; i++) {
      positions[posIndex][i] = servoPos[i];
    }
    posIndex++;
    Serial.println("Position Recorded");
    buzzerBeep();  // Indicate recording with a beep
    delay(300);  // Debounce delay
  }

  // Check playback button state
  if (digitalRead(playbackButtonPin) == LOW) {
    Serial.println("Playing back recorded positions...");
    playbackRecordedPositions();
    delay(300);  // Debounce delay
  }

  // Update motor positions
  if (currentState == 1) {
    servoPos[0] += (yVal > 600) ? 3 : (yVal < 400) ? -3 : 0;
    servoPos[1] += (xVal > 600) ? 3 : (xVal < 400) ? -3 : 0;
  } else {
    servoPos[2] += (yVal > 600) ? 7 : (yVal < 400) ? -7 : 0;
    servoPos[3] += (xVal > 600) ? 3 : (xVal < 400) ? -3 : 0;
  }

  // Limit servo positions (0 to 180 degrees)
  for (int i = 0; i < 4; i++) {
    servoPos[i] = constrain(servoPos[i], 0, 180);
  }

  // Set servo motor positions
  setServoPosition(servo1, servoPos[0]);
  setServoPosition(servo2, servoPos[1]);
  setServoPosition(servo3, servoPos[2]);
  setServoPosition(servo4, servoPos[3]);

  // Print values to the console
  Serial.print("X: ");
  Serial.print(xVal);
  Serial.print(" Y: ");
  Serial.print(yVal);
  Serial.print(" Joystick Button: ");
  Serial.print(digitalRead(joystickButtonPin));
  Serial.print(" Record Button: ");
  Serial.print(digitalRead(recordButtonPin));
  Serial.print(" Playback Button: ");
  Serial.print(digitalRead(playbackButtonPin));
  Serial.print(" Current State: ");
  Serial.print(currentState);
  Serial.print(" Servo Positions: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(servoPos[i]);
    if (i < 3) Serial.print(", ");
  }
  Serial.println();

  delay(10);  // Small delay
}
