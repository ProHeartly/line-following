// I'm not sure if I'm going to use esp32 devkit or arduino UNO for the actual project.. so I'm going to keep part for both of them :p


// arduino---------------------------------------------------------------
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

float Kp = 35.0;
float Ki = 0.0;
float Kd = 20.0;

int baseSpeed = 160;
int maxSpeed = 230;
int lastError = 0;


void setup() {
  Serial.begin(9600);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  
}

void loop() {
  // put your main code here, to run repeatedly:
}
