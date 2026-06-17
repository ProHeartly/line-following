// I'm not sure if I'm going to use esp32 devkit or arduino UNO for the actual project.. so I'm going to keep part for both of them :p
// Imma use #ifdef thingy for using same code for esp and arduino.

// #define USE_ESP32 // Uncomment this line for esp32

// -------------- CONFIG ---------------
#ifdef USE_ESP32
#include <WiFi.h>
#include <WebServer.h>

#define S1 32
#define S2 33
#define S3 34
#define S4 35
#define S5 36

#define ENA 18
#define ENB 19
#define IN1 5
#define IN2 17
#define IN3 16
#define IN4 4

const char* ssid = "pidish";
const char* password = "aromatic";
WebServer server(80);

#else
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
#endif

// ----------- VARIABLES ------------

float Kp = 35.0;
float Ki = 0.0;
float Kd = 20.0;

int baseSpeed = 160;
int maxSpeed = 200;
int lastError = 0;

int igl = 0;
int maxIgl = 100;


// WEBSERVER (only for esp32)
#ifdef USE_ESP32
const char HTML_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>PID Tuner</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: monospace; background: #0d1117; color: #c9d1d9; padding: 1.5rem; }
  h1 { font-size: 1rem; color: #8b949e; letter-spacing: 0.1em; text-transform: uppercase; margin-bottom: 1.5rem; }
  .card { background: #161b22; border: 1px solid #30363d; border-radius: 8px; padding: 1rem 1.25rem; margin-bottom: 1rem; }
  .card h2 { font-size: 0.7rem; color: #8b949e; letter-spacing: 0.08em; text-transform: uppercase; margin-bottom: 1rem; }
  .field { display: flex; align-items: center; gap: 12px; margin-bottom: 0.75rem; }
  .field label { min-width: 90px; font-size: 0.8rem; color: #8b949e; }
  .field input[type=number] {
    flex: 1; background: #0d1117; border: 1px solid #30363d; border-radius: 6px;
    color: #58a6ff; font-family: monospace; font-size: 1rem; padding: 6px 10px;
  }
  .field input[type=number]:focus { outline: none; border-color: #58a6ff; }
  .field .unit { font-size: 0.75rem; color: #484f58; min-width: 20px; }
  .btn-row { display: flex; gap: 10px; margin-top: 0.5rem; }
  button {
    flex: 1; padding: 10px; border-radius: 6px; font-family: monospace;
    font-size: 0.85rem; cursor: pointer; border: 1px solid #30363d;
  }
  #btnReset { background: #161b22; color: #8b949e; }
  #btnSave  { background: #1f4a2e; border-color: #3fb950; color: #3fb950; }
  #toast {
    display: none; position: fixed; bottom: 1.5rem; left: 50%; transform: translateX(-50%);
    background: #1f4a2e; border: 1px solid #3fb950; color: #3fb950;
    padding: 8px 20px; border-radius: 6px; font-size: 0.8rem;
  }
</style>
</head><body>
<h1>PID Tuner</h1>

<div class="card">
  <h2>PID parameters</h2>
  <div class="field"><label>Kp</label><input type="number" id="kp" step="0.5" value="__KP__"><span class="unit"></span></div>
  <div class="field"><label>Ki</label><input type="number" id="ki" step="0.01" value="__KI__"><span class="unit"></span></div>
  <div class="field"><label>Kd</label><input type="number" id="kd" step="0.5" value="__KD__"><span class="unit"></span></div>
</div>

<div class="card">
  <h2>Speed settings</h2>
  <div class="field"><label>Base speed</label><input type="number" id="base" step="5" min="0" max="255" value="__BASE__"><span class="unit">/255</span></div>
  <div class="field"><label>Max speed</label><input type="number" id="maxs" step="5" min="0" max="255" value="__MAX__"><span class="unit">/255</span></div>
</div>

<div class="btn-row">
  <button id="btnReset" onclick="resetDefaults()">reset defaults</button>
  <button id="btnSave"  onclick="saveValues()">apply & save</button>
</div>

<div id="toast">saved!</div>

<script>
  const DEFAULTS = { kp:35, ki:0, kd:20, base:160, maxs:200 };

  function resetDefaults() {
    Object.entries(DEFAULTS).forEach(([k,v]) => document.getElementById(k).value = v);
  }

  function saveValues() {
    const params = new URLSearchParams({
      kp:   document.getElementById('kp').value,
      ki:   document.getElementById('ki').value,
      kd:   document.getElementById('kd').value,
      base: document.getElementById('base').value,
      maxs: document.getElementById('maxs').value,
    });
    fetch('/set?' + params)
      .then(r => r.text())
      .then(() => {
        const t = document.getElementById('toast');
        t.style.display = 'block';
        setTimeout(() => t.style.display = 'none', 1800);
      })
      .catch(() => alert('Error reaching the bot — check WiFi!'));
  }
</script>
</body></html>
)rawhtml";

void handleRoot() {
  String page = HTML_PAGE;
  page.replace("__KP__", String(Kp));
  page.replace("__KI__", String(Ki));
  page.replace("__KD__", String(Kd));
  page.replace("__BASE__", String(baseSpeed));
  page.replace("__MAX__", String(maxSpeed));
  server.send(200, "text/html", page);
}

void handleSet() {
  if (server.hasArg("kp"))   Kp        = server.arg("kp").toFloat();
  if (server.hasArg("ki"))   Ki        = server.arg("ki").toFloat();
  if (server.hasArg("kd"))   Kd        = server.arg("kd").toFloat();
  if (server.hasArg("base")) baseSpeed = server.arg("base").toInt();
  if (server.hasArg("maxs")) maxSpeed  = server.arg("maxs").toInt();

  // reset integral whenever gains change so it doesn't kick
  igl = 0;

  server.send(200, "text/plain", "ok");
}
#endif

void setup() {
  #ifdef USE_ESP32
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.println("connecting to network...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin();
    Serial.println("Web server started :D");
  #else
    Serial.begin(9600);
  #endif

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
  #ifdef USE_ESP32
    server.handleClient(); // this so that it runs every loop :p
  #endif

  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  int error = 0;

  // initial logic. NOTE: I MIGHT CHANGE THIS IN FUTURE cuz IK MY LOGIC ISN'T damn SOLID
  // - THis when bot is dead straight -
  if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 0 && s5 == 0) error = 0;

  // - left -
  else if (s1 == 0 && s2 == 1 && s3 == 1 && s4 == 0 && s5 == 0) error = -1;  // SLIGHT LEFT
  else if (s1 == 0 && s2 == 1 && s3 == 0 && s4 == 0 && s5 == 0) error = -2;  // little LEFT
  else if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 0 && s5 == 0) error = -3;  // mid LEFT
  else if (s1 == 1 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 0) error = -4;  // HARD LEFT

  // - right -
  else if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 1 && s5 == 0) error = 1;  // SLIGHT RIGHT
  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 1 && s5 == 0) error = 2;  // little RIGHT
  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 1 && s5 == 1) error = 3;  // mid RIGHT
  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 1) error = 4;  // HARD RIgHT

  // - intersection -
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1 && s5 == 1) error = 0;

  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 0) {
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
}

void moveMotors(int leftMotorSpeed, int rightMotorSpeed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, leftMotorSpeed);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, rightMotorSpeed);
}
