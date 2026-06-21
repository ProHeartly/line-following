// I'm not sure if I'm going to use esp32 devkit or arduino UNO for the actual project.. so I'm going to keep part for both of them :p
// Imma use #ifdef thingy for using same code for esp and arduino.
// NVM we are going to use UNO so lets remove esp32 block

// #define USE_ESP32 // Uncomment this line for esp32

// -------------- CONFIG ---------------

#define S1 A0
#define S2 A1
#define S3 A2
#define S4 A3
#define S5 A4

#define ENA 5
#define ENB 6
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 10

// ----------- VARIABLES ------------

float Kp = 10.0;
float Ki = 0.0;
float Kd = 17.0;

int baseSpeed = 70;
int maxSpeed = 200;
int lastError = 0;

int igl = 0;
int maxIgl = 100;

void setup() {
  Serial.begin(9600);

  // Sensors
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);

  // Motor
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // forward initially
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  // RANDMO SERIAL MSG - not really random thooo.. I LOVE PROJECT: HAIL MARRY!!!
  Serial.println("Amaze Amaze Amaze!!!");
  }

void loop() {
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  int error = 0;

  // initial logic. NOTE: I MIGHT CHANGE THIS IN FUTURE cuz IK MY LOGIC ISN'T damn SOLID
  // - THis when bot is dead straight -
  if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 1 && s5 == 1) error = 0;

  // - left -
  else if (s1 == 1 && s2 == 0 && s3 == 0 && s4 == 1 && s5 == 1) error = -1;  // SLIGHT LEFT
  else if (s1 == 1 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 1) error = -2;  // little LEFT
  else if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 1) error = -3;  // mid LEFT
  else if (s1 == 0 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 1) error = -4;  // HARD LEFT

  // - right -
  else if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 0 && s5 == 1) error = 1;  // SLIGHT RIGHT
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 1) error = 2;  // little RIGHT
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 0) error = 3;  // mid RIGHT
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 0) error = 4;  // HARD RIgHT

  // - intersection -
  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 0) error = 0;

  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 1) {
    if (lastError < 0) {
      error = -5;
    } else if (lastError > 0) {
      error = 5;
    }
  }

  // - PID calculation -
  int p = error;
  if (Ki != 0) {
    igl = igl + error;  // I added integral's logic just in case if I need to use :D
    igl = constrain(igl, -maxIgl, maxIgl);
  }
  int d = error - lastError;

  float motorSpeedAdjustment = (Kp * p) + (Ki * igl) + (Kd * d);
  lastError = error;

  int leftMotorSpeed = baseSpeed + (int)motorSpeedAdjustment;
  int rightMotorSpeed = baseSpeed - (int)motorSpeedAdjustment;

  leftMotorSpeed = constrain(leftMotorSpeed, 0, maxSpeed);
  rightMotorSpeed = constrain(rightMotorSpeed, 0, maxSpeed);

  // - Motor movement -
  moveMotors(leftMotorSpeed, rightMotorSpeed);
  delay(5);
}

void moveMotors(int leftMotorSpeed, int rightMotorSpeed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, leftMotorSpeed);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, rightMotorSpeed);
}
