#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_W 128
#define OLED_H 64
Adafruit_SSD1306 display(OLED_W, OLED_H, &Wire, -1);

// TB6612FNG
const uint8_t PWMA=5, AIN1=7, AIN2=8;
const uint8_t PWMB=6, BIN1=9, BIN2=10;
const uint8_t STBY=4;

// 74HC4051
const uint8_t MUX_SIG=A0, MUX_S0=11, MUX_S1=12, MUX_S2=13;

// Buttons
const uint8_t BTN_START=2, BTN_MODE=3, BTN_MINUS=A1;

const uint8_t N=8;
const int16_t position[N]={-3500,-2500,-1500,-500,500,1500,2500,3500};
const bool SENSOR_REVERSED=false;
const bool BLACK_IS_HIGH=true;

uint16_t sensorMin[N], sensorMax[N];
int raw[N], sensor[N];
long weightedSum=0, sensorSum=0;
int lastPosition=0;

float KP=0.105f;
float KI=0.0008f;
float KD=0.82f;
float integral=0, lastError=0, filteredD=0;

const float D_FILTER=0.28f;
const float I_LIMIT=9000.0f;
const float I_ZONE=1200.0f;

int BASE_SPEED=155;
const int MIN_SPEED=65, MAX_SPEED=245;
const float TURN_SLOWDOWN=0.055f;
float STEERING_BIAS=0.0f;
int LEFT_TRIM=0, RIGHT_TRIM=0;

int lastLeftPWM=0, lastRightPWM=0;
const int PWM_STEP=9;

bool running=false, calibrated=false;
uint32_t lastLoopMicros=0, lastDisplay=0, lastButtonMs=0;

void muxSelect(uint8_t ch){
  digitalWrite(MUX_S0,ch&1);
  digitalWrite(MUX_S1,(ch>>1)&1);
  digitalWrite(MUX_S2,(ch>>2)&1);
}

int readMux(uint8_t ch){
  muxSelect(ch);
  delayMicroseconds(3);
  return analogRead(MUX_SIG);
}

void readSensors(){
  for(uint8_t i=0;i<N;i++){
    uint8_t ch=SENSOR_REVERSED ? (N-1-i) : i;
    raw[i]=readMux(ch);
    long den=(long)sensorMax[i]-sensorMin[i];
    if(den<8) den=8;
    long v;
    if(BLACK_IS_HIGH) v=(long)(raw[i]-sensorMin[i])*1000L/den;
    else v=(long)(sensorMax[i]-raw[i])*1000L/den;
    sensor[i]=constrain((int)v,0,1000);
  }
}

int calculatePosition(){
  weightedSum=0;
  sensorSum=0;
  for(uint8_t i=0;i<N;i++){
    weightedSum+=(long)sensor[i]*position[i];
    sensorSum+=sensor[i];
  }
  if(sensorSum<120) return lastPosition<0 ? -4000 : 4000;
  int p=(int)(weightedSum/sensorSum);
  lastPosition=p;
  return p;
}

int slew(int target,int current){
  if(target>current+PWM_STEP) return current+PWM_STEP;
  if(target<current-PWM_STEP) return current-PWM_STEP;
  return target;
}

void setMotor(uint8_t pwm,uint8_t in1,uint8_t in2,int speed){
  speed=constrain(speed,-255,255);
  if(speed>0){digitalWrite(in1,HIGH);digitalWrite(in2,LOW);analogWrite(pwm,speed);}
  else if(speed<0){digitalWrite(in1,LOW);digitalWrite(in2,HIGH);analogWrite(pwm,-speed);}
  else {digitalWrite(in1,LOW);digitalWrite(in2,LOW);analogWrite(pwm,0);}
}

void stopMotors(){
  lastLeftPWM=lastRightPWM=0;
  setMotor(PWMA,AIN1,AIN2,0);
  setMotor(PWMB,BIN1,BIN2,0);
}

void drive(int left,int right){
  left=constrain(left+LEFT_TRIM,-255,255);
  right=constrain(right+RIGHT_TRIM,-255,255);
  lastLeftPWM=slew(left,lastLeftPWM);
  lastRightPWM=slew(right,lastRightPWM);
  setMotor(PWMA,AIN1,AIN2,lastLeftPWM);
  setMotor(PWMB,BIN1,BIN2,lastRightPWM);
}

void calibrationInit(){
  for(uint8_t i=0;i<N;i++){sensorMin[i]=1023;sensorMax[i]=0;}
}

void calibrationSample(){
  for(uint8_t i=0;i<N;i++){
    uint8_t ch=SENSOR_REVERSED ? (N-1-i) : i;
    int v=readMux(ch);
    if(v<sensorMin[i]) sensorMin[i]=v;
    if(v>sensorMax[i]) sensorMax[i]=v;
  }
}

void autoCalibrate(uint32_t ms){
  calibrationInit();
  uint32_t start=millis();
  while(millis()-start<ms){
    calibrationSample();
    display.clearDisplay();
    display.setCursor(0,0);
    display.println(F("CALIBRATING"));
    display.println(F("Move sensors over"));
    display.println(F("WHITE + BLACK"));
    int w=map((long)(millis()-start),0,ms,0,116);
    display.drawRect(4,45,120,10,SSD1306_WHITE);
    display.fillRect(6,47,constrain(w,0,116),6,SSD1306_WHITE);
    display.display();
  }
  calibrated=true;
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("CAL OK"));
  display.display();
  delay(400);
}

void updatePID(){
  uint32_t now=micros();
  float dt=(lastLoopMicros==0)?0.0025f:(now-lastLoopMicros)*1e-6f;
  lastLoopMicros=now;
  dt=constrain(dt,0.0005f,0.020f);

  int positionError=calculatePosition();
  float error=(float)positionError+STEERING_BIAS;
  bool lineLost=(sensorSum<120);

  if(!lineLost && fabs(error)<I_ZONE){
    integral+=error*dt;
    integral=constrain(integral,-I_LIMIT,I_LIMIT);
  }else{
    integral*=0.94f;
  }

  float derivative=(error-lastError)/dt;
  filteredD+=D_FILTER*(derivative-filteredD);
  float correction=KP*error+KI*integral+KD*filteredD;

  float n=min(fabs(error)/3500.0f,1.5f);
  int dynamicBase=BASE_SPEED-(int)(TURN_SLOWDOWN*255.0f*n*n);
  dynamicBase=constrain(dynamicBase,MIN_SPEED,MAX_SPEED);

  int leftTarget,rightTarget;
  if(lineLost){
    int search=(lastPosition<0)?-110:110;
    leftTarget=search; rightTarget=-search;
  }else{
    correction=constrain(correction,-255.0f,255.0f);
    leftTarget=dynamicBase+(int)correction;
    rightTarget=dynamicBase-(int)correction;
  }
  drive(leftTarget,rightTarget);
  lastError=error;
}

void drawOLED(int p){
  if(millis()-lastDisplay<100) return;
  lastDisplay=millis();
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("PID "));display.print(running?F("RUN"):F("STOP"));
  display.print(F(" P:"));display.print(p);
  display.setCursor(0,12);
  display.print(F("K "));display.print(KP,3);display.print(' ');
  display.print(KI,4);display.print(' ');display.print(KD,2);
  display.setCursor(0,24);
  display.print(F("PWM "));display.print(lastLeftPWM);display.print('/');
  display.print(lastRightPWM);
  display.setCursor(0,36);
  display.print(F("S "));
  for(uint8_t i=0;i<N;i++) display.print(sensor[i]>600?'#':(sensor[i]>250?'+':'.'));
  display.setCursor(0,48);
  display.print(F("SPD "));display.print(BASE_SPEED);
  display.print(F(" I "));display.print((int)integral);
  display.display();
}

bool pressed(uint8_t pin){
  if(digitalRead(pin)==LOW && millis()-lastButtonMs>220){
    lastButtonMs=millis();
    return true;
  }
  return false;
}

void handleButtons(){
  if(pressed(BTN_START)){
    running=!running;
    if(!running){stopMotors();integral=0;filteredD=0;}
  }
  if(pressed(BTN_MODE)){
    // Re-run calibration on MODE press.
    running=false;
    stopMotors();
    autoCalibrate(1500);
  }
  if(pressed(BTN_MINUS)){
    BASE_SPEED-=5;
    if(BASE_SPEED<MIN_SPEED) BASE_SPEED=MIN_SPEED;
  }
}

void setup(){
  pinMode(PWMA,OUTPUT);pinMode(AIN1,OUTPUT);pinMode(AIN2,OUTPUT);
  pinMode(PWMB,OUTPUT);pinMode(BIN1,OUTPUT);pinMode(BIN2,OUTPUT);
  pinMode(STBY,OUTPUT);
  pinMode(MUX_S0,OUTPUT);pinMode(MUX_S1,OUTPUT);pinMode(MUX_S2,OUTPUT);
  pinMode(BTN_START,INPUT_PULLUP);pinMode(BTN_MODE,INPUT_PULLUP);pinMode(BTN_MINUS,INPUT_PULLUP);
  digitalWrite(STBY,HIGH);
  stopMotors();

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("PID LINE FOLLOWER"));
  display.println(F("8CH + 4051"));
  display.println(F("CALIBRATING..."));
  display.display();

  autoCalibrate(2500);
  lastLoopMicros=micros();
}

void loop(){
  handleButtons();
  readSensors();
  int p=calculatePosition();

  if(running && calibrated) updatePID();
  else stopMotors();

  drawOLED(p);
}
