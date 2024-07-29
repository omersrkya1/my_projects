#include <Servo.h>

const int gripperServoPin = 3;
const int upDownServoPin = 11;
const int leftRightServoPin = 9;
const int otherServoPin = 10;
const int buzzerPin = 6; // Buzzer için pin tanımı

const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickButtonPin = 2;
const int recordButtonPin = 4;
const int playButtonPin = 5;

Servo gripperServo;
Servo upDownServo;
Servo leftRightServo;
Servo otherServo;

int gripperAngle = 90;
int upDownAngle = 90;
int leftRightAngle = 90;
int otherAngle = 90;

int currentState = 1; // İlk durumda başla
bool buttonPressed = false; // Joystick butonunun basılı olup olmadığını takip eder
bool recordButtonPressed = false; // Kayıt butonunun basılı olup olmadığını takip eder
bool playButtonPressed = false; // Oynatma butonunun basılı olup olmadığını takip eder
const int threshold = 50; // Joystick değerleri için hassasiyet eşiği
const int recordDelay = 3000; // Hareket kayıt süresi (ms)
const int playDelay = 10; // Hareket oynatma gecikme süresi (ms)
const int maxRecordings = 10; // Maksimum kayıt sayısı

struct ServoState {
  int gripper;
  int upDown;
  int leftRight;
  int other;
};

ServoState recordedStates[maxRecordings]; // Kaydedilen servo durumu dizisi
int numRecordings = 0; // Kaydedilen hareket sayısı

void setup() {
  pinMode(joystickButtonPin, INPUT_PULLUP); // Joystick butonu için pull-up direnci etkinleştirilir
  pinMode(recordButtonPin, INPUT_PULLUP); // Kayıt butonu için pull-up direnci etkinleştirilir
  pinMode(playButtonPin, INPUT_PULLUP); // Oynatma butonu için pull-up direnci etkinleştirilir
  pinMode(buzzerPin, OUTPUT); // Buzzer için çıkış tanımlaması

  // Servo motorları başlat
  gripperServo.attach(gripperServoPin);
  upDownServo.attach(upDownServoPin);
  leftRightServo.attach(leftRightServoPin);
  otherServo.attach(otherServoPin);

  gripperServo.write(gripperAngle);
  upDownServo.write(upDownAngle);
  leftRightServo.write(leftRightAngle);
  otherServo.write(otherAngle);

  Serial.begin(9600); // Seri haberleşme başlat
}

void loop() {
  // Joystick butonunun durumunu kontrol et
  if (digitalRead(joystickButtonPin) == LOW) {
    if (!buttonPressed) {
      currentState = (currentState == 1) ? 2 : 1; // Durumu değiştir
      Serial.print("Button Pressed: Mode Change -> State: ");
      Serial.println(currentState);
      buttonPressed = true;
    }
  } else {
    buttonPressed = false;
  }

  // Kayıt butonunun durumunu kontrol et
  if (digitalRead(recordButtonPin) == LOW) {
    if (!recordButtonPressed) {
      if (numRecordings < maxRecordings) {
        recordMovement(numRecordings);
        numRecordings++;
      } else {
        Serial.println("Max recordings reached!");
      }
      recordButtonPressed = true;
    }
  } else {
    recordButtonPressed = false;
  }

  // Oynatma butonunun durumunu kontrol et
  if (digitalRead(playButtonPin) == LOW) {
    if (!playButtonPressed) {
      for (int i = 0; i < numRecordings; ++i) {
        playRecordedMovement(recordedStates[i]);
      }
      playButtonPressed = true;
    }
  } else {
    playButtonPressed = false;
  }

  // Joystick hareketlerini oku ve servo pozisyonlarını güncelle
  int joystickXValue = analogRead(joystickXPin);
  int joystickYValue = analogRead(joystickYPin);

  if (currentState == 1) { // İlk durum, 1. ve 2. motorları kontrol eder
    updateServoPositions(gripperServo, gripperAngle, upDownServo, upDownAngle, joystickXValue, joystickYValue);
    Serial.print("Gripper Angle: ");
    Serial.print(gripperAngle);
    Serial.print(", Up-Down Angle: ");
    Serial.println(upDownAngle);
  } else { // İkinci durum, 3. ve 4. motorları kontrol eder
    updateServoPositions(leftRightServo, leftRightAngle, otherServo, otherAngle, joystickXValue, joystickYValue);
    Serial.print("Left-Right Angle: ");
    Serial.print(leftRightAngle);
    Serial.print(", Other Angle: ");
    Serial.println(otherAngle);
  }

  delay(50); // Küçük bir gecikme ekleyerek servo hareketlerini yumuşat
}

// Servo pozisyonlarını günceller
void updateServoPositions(Servo & servo1, int & angle1, Servo & servo2, int & angle2, int joyX, int joyY) {
  if (joyY > 512 + threshold) {
    angle1 += 4;
    if (angle1 > 180) angle1 = 180;
  } else if (joyY < 512 - threshold) {
    angle1 -= 4;
    if (angle1 < 0) angle1 = 0;
  }
  if (joyX > 512 + threshold) {
    angle2 += 4;
    if (angle2 > 180) angle2 = 180;
  } else if (joyX < 512 - threshold) {
    angle2 -= 4;
    if (angle2 < 0) angle2 < 0;
  }
  servo1.write(angle1);
  servo2.write(angle2);
}

// Servo durumunu kaydeder
void recordMovement(int index) {
  recordedStates[index].gripper = gripperAngle;
  recordedStates[index].upDown = upDownAngle;
  recordedStates[index].leftRight = leftRightAngle;
  recordedStates[index].other = otherAngle;
  Serial.print("Movement ");
  Serial.print(index + 1);
  Serial.println(" Recorded");
  
  // Buzzer'ı 1 kez çaldır
  tone(buzzerPin, 1000);  // İstenilen frekansı ayarlayın
  delay(200);  // Buzzer'ın ne kadar süreyle çalacağını belirtin
  noTone(buzzerPin);  // Buzzer'ı durdur
}

// Kaydedilen servo durumunu interpolasyonla oynatır
void playRecordedMovement(ServoState state) {
  int startGripper = gripperAngle;
  int startUpDown = upDownAngle;
  int startLeftRight = leftRightAngle;
  int startOther = otherAngle;

  // Buzzer'ı 3 kez kısa süreli çaldır
  for (int i = 0; i < 3; ++i) {
    tone(buzzerPin, 1500);  // Farklı bir frekansta çalmasını sağlayın
    delay(100);  // Kısa süreyle çal
    noTone(buzzerPin);  // Buzzer'ı durdur
    delay(100);  // Bir sonraki çalım arasında kısa bir gecikme
  }

  for (float t = 0.0; t <= 1.0; t += 0.01) {
    gripperAngle = startGripper + (state.gripper - startGripper) * t;
    upDownAngle = startUpDown + (state.upDown - startUpDown) * t;
    leftRightAngle = startLeftRight + (state.leftRight - startLeftRight) * t;
    otherAngle = startOther + (state.other - startOther) * t;

    gripperServo.write(gripperAngle);
    upDownServo.write(upDownAngle);
    leftRightServo.write(leftRightAngle);
    otherServo.write(otherAngle);

    delay(playDelay);
  }
  Serial.println("Movement Playback Complete");
}
