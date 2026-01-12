#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ap_ssid = "ESP32-vehicle-control";
const char* ap_password = "12345678";

WebServer server(80);

const int LeftMotorForward  = 27;   
const int LeftMotorBackward = 26;   
const int RightMotorForward = 25;   
const int RightMotorBackward= 33;   

#define TRIG_PIN 32
#define ECHO_PIN 35
#define LED_PIN 2  

boolean goesForward = false;
int distance = 100;
int speedMotor = 200;
bool autoMode = true;

Servo servo_motor;

void setup() {
  Serial.begin(115200);

  pinMode(RightMotorForward, OUTPUT);
  pinMode(LeftMotorForward, OUTPUT);
  pinMode(LeftMotorBackward, OUTPUT);
  pinMode(RightMotorBackward, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  servo_motor.attach(13, 500, 2400); 
  servo_motor.write(90);

  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("ESP32 da tao WiFi AP");
  Serial.print("SSID: "); Serial.println(ap_ssid);
  Serial.print("PASS: "); Serial.println(ap_password);
  Serial.print("IP: "); Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/forward", [](){ moveForward(); server.send(200, "text/plain", "Forward"); });
  server.on("/backward", [](){ moveBackward(); server.send(200, "text/plain", "Backward"); });
  server.on("/left", [](){ turnLeft(); server.send(200, "text/plain", "Left"); });
  server.on("/right", [](){ turnRight(); server.send(200, "text/plain", "Right"); });
  server.on("/stop", [](){ moveStop(); server.send(200, "text/plain", "Stop"); });
  server.on("/auto", [](){ autoMode = true; server.send(200, "text/plain", "Auto Mode"); });
  server.on("/manual", [](){ autoMode = false; moveStop(); server.send(200, "text/plain", "Manual Mode"); });

  server.on("/speed", [](){
    if(server.hasArg("val")){
      speedMotor = server.arg("val").toInt();
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/led", [](){
    if(distance <= 30) server.send(200, "text/plain", "ON");
    else server.send(200, "text/plain", "OFF");
  });

  server.begin();
  Serial.println("WebServer da san sang!");
}

void loop() {
  server.handleClient();

  distance = readPing();
  if (distance <= 30) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  if (autoMode) {
    runAuto();
  }
}

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { background:#202020; color:white; font-family:Arial; text-align:center; }
      .joystick {
        width:200px; height:200px;
        background:#444; border-radius:50%;
        margin:20px; display:inline-block;
        position:relative; touch-action:none;
      }
      .stick {
        width:80px; height:80px;
        background:#1e90ff; border-radius:50%;
        position:absolute; top:60px; left:60px;
      }
      #buttons { margin-top:30px; }
      button { width:120px; height:50px; margin:10px; font-size:18px; border-radius:12px; border:none; }
      .mode { background:#ff9800; color:white; }
      .stop { background:red; color:white; font-weight:bold; }
      #led { width:30px; height:30px; border-radius:50%; display:inline-block; margin:10px; background:green; }
      #speedValue { font-size:18px; margin-top:10px; }
    </style>
  </head>
  <body>
    <h2>Control ESP32</h2>
    
    <div>
      <div id="joystick1" class="joystick"><div id="stick1" class="stick"></div></div>
      <div id="joystick2" class="joystick"><div id="stick2" class="stick"></div></div>
    </div>

    <div id="buttons">
      <button class="mode" onclick="sendCmd('auto')">Auto Mode</button>
      <button class="mode" onclick="sendCmd('manual')">Manual Mode</button><br>
      <button class="stop" onclick="sendCmd('stop')">STOP</button>
    </div>

    <h3>Entity Detective</h3>
    <div id="led"></div>

    <h3>Speed Control</h3>
    <input type="range" min="0" max="255" value="200" id="speedSlider" onchange="updateSpeed(this.value)">
    <div id="speedValue">Speed: 200</div>

    <script>
      function setupJoystick(joyId, stickId, axis) {
        let joy = document.getElementById(joyId);
        let stick = document.getElementById(stickId);
        let centerX = joy.offsetWidth/2, centerY = joy.offsetHeight/2;
        let active=false, currentCmd="stop";
        
        joy.addEventListener("touchstart", e=>{
          active=true; e.preventDefault();
        });
        
        joy.addEventListener("touchend", e=>{
          active=false; 
          stick.style.left="60px"; 
          stick.style.top="60px";
          sendCmd("stop"); 
          currentCmd="stop";
        });
        
        joy.addEventListener("touchmove", e=>{
          if(!active) return;
          let rect=joy.getBoundingClientRect();
          let x=e.touches[0].clientX-rect.left, 
              y=e.touches[0].clientY-rect.top;
          let dx=x-centerX, dy=y-centerY;
          let dist=Math.sqrt(dx*dx+dy*dy), maxDist=70;
          if(dist>maxDist){ dx*=maxDist/dist; dy*=maxDist/dist; }
          stick.style.left=(centerX+dx-stick.offsetWidth/2)+"px";
          stick.style.top =(centerY+dy-stick.offsetHeight/2)+"px";
          
          let cmd="stop";
          if(axis==="Y"){  
            if(dy<-20) cmd="forward";
            else if(dy>20) cmd="backward";
          } else {         
            if(dx<20) cmd="right";
            else if(dx>-20) cmd="left";
          }
          
          if(cmd!==currentCmd){ 
            currentCmd=cmd; 
            sendCmd(cmd); 
          }
        });
      }
      
      setupJoystick("joystick1","stick1","Y");
      setupJoystick("joystick2","stick2","X");

      function sendCmd(cmd){
        fetch("/"+cmd).catch(err=>console.log(err));
      }

      function updateSpeed(val){
        document.getElementById("speedValue").innerText = "Speed: " + val;
        fetch("/speed?val="+val);
      }

      setInterval(()=>{
        fetch("/led").then(res=>res.text()).then(state=>{
          let led=document.getElementById("led");
          if(state=="ON") led.style.background="red";
          else led.style.background="green";
        });
      },500);
    </script>
  </body>
  </html>
  )rawliteral";
  server.send(200, "text/html", html);
}


void runAuto() {
  int distanceRight = 0;
  int distanceLeft = 0;
  delay(50);

  distance = readPing();
  if (distance <= 30) {
    moveStop();
    delay(200);
    moveBackward();
    delay(400);
    moveStop();
    delay(200);

    distanceRight = lookRight();
    distanceLeft = lookLeft();

    if (distanceRight >= distanceLeft) {
      turnRight();
      delay(400);
    } else {
      turnLeft();
      delay(400);
    }
    moveStop();
  } else {
    moveForward();
  }
}

int lookRight() {
  servo_motor.write(0);
  delay(400);
  int d = readPing();
  servo_motor.write(90);
  delay(150);
  return d;
}

int lookLeft() {
  servo_motor.write(180);
  delay(400);
  int d = readPing();
  servo_motor.write(90);
  delay(150);
  return d;
}

int readPing() {
  long duration;
  int cm;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  cm = duration * 0.034 / 2;
  if (cm == 0) cm = 250;
  return cm;
}

void moveStop() {
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorBackward, LOW);
  digitalWrite(LeftMotorBackward, LOW);
}

void moveForward() {
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(RightMotorForward, LOW);
}

void moveBackward() {
  digitalWrite(LeftMotorBackward, HIGH);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(RightMotorBackward, LOW);
}

void turnRight() {
  digitalWrite(LeftMotorForward, HIGH);
  digitalWrite(LeftMotorBackward, LOW);
  digitalWrite(RightMotorForward, HIGH);
  digitalWrite(RightMotorBackward, LOW);
}

void turnLeft() {
  digitalWrite(RightMotorBackward, HIGH);
  digitalWrite(RightMotorForward, LOW);
  digitalWrite(LeftMotorForward, LOW);
  digitalWrite(LeftMotorBackward, HIGH);
}
