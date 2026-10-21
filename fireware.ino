#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Servo.h>  

//  Display & Communication 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // 0x27, 16 cols, 2 rows
SoftwareSerial co2Serial(4, 5);      // CO2 Sensor SoftwareSerial (RX=4, TX=5)

//  Original Sensors & Basic Alarm Pins 
const int PM_PIN = A0;       // PM2.5 Sensor Analog IN
const int MQ7_PIN = A2;      // CO Sensor Analog IN
const int HEATER_PIN = 9;    // MQ-7 Internal Heater Control
const int PM_LED_PIN = 3;    // PM2.5 Sensor IR LED Control
const int BUZZER_PIN = 10;   // Buzzer Alarm
const int LED_PIN = 11;      // Warning LED
const int FAN_RELAY_PIN = 12; // Intake Fan Relay Control

//  Patent Actuator Pins (Tesla Valve & Purification) 
const int PUMP_RELAY_PIN = 13;      // Micro Pump Relay (H2O2 injection)
const int ACTUATOR_IN1 = 7;         // Linear Actuator Extend (Expand cross-section)
const int ACTUATOR_IN2 = 8;         // Linear Actuator Retract
const int BAFFLE_SERVO_PIN = 6;     // Tesla Valve Baffle Servo
const int CATALYTIC_HEATER_PIN = 2; // High-temp Catalytic Chamber PWM Temp Control

//  Safety Thresholds & Sensor Parameters 
const float PM25_GOOD_MAX = 12.0;  // PM2.5 Good Air Quality Max (ug/m3)
const float PM25_SAFE_MAX = 35.0;  // PM2.5 Safe Concentration Max (ug/m3)
const float CO_GOOD_MAX = 9.0;     // CO Good Air Quality Max (ppm)
const float CO_SAFE_MAX = 35.0;    // CO Safe Concentration Max (ppm)

#define RL_VALUE 10.0          // Load Resistance (kOhm)
#define RO_CLEAN_AIR 9.8       // Sensor Resistance in Clean Air (kOhm)
#define CO2_PREHEAT_MS 300000  // CO2 Sensor Preheat Time (ms)

//  System Control Variables 
unsigned long alarmEndTime = 0;    // Timestamp for alarm delay end
Servo baffleServo;                 // Servo object
bool isTeslaExpanded = false;      // Tesla Valve state lock (prevents actuator from stuck/repeated powering)


void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);

  // Initialize Sensor Hardware Pins
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(PM_LED_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, HIGH);  
  digitalWrite(PM_LED_PIN, HIGH);  

  // Initialize Alarm & Actuator Pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(ACTUATOR_IN1, OUTPUT);
  pinMode(ACTUATOR_IN2, OUTPUT);
  pinMode(CATALYTIC_HEATER_PIN, OUTPUT);
  
  baffleServo.attach(BAFFLE_SERVO_PIN);
  
  // Initial State Setup (All OFF / Zero Position / Energy Saving)
  digitalWrite(BUZZER_PIN, HIGH);    // Buzzer OFF (Active LOW)
  digitalWrite(LED_PIN, LOW);        
  digitalWrite(FAN_RELAY_PIN, LOW);  
  digitalWrite(PUMP_RELAY_PIN, LOW); 
  digitalWrite(ACTUATOR_IN1, LOW);   
  digitalWrite(ACTUATOR_IN2, LOW);
  analogWrite(CATALYTIC_HEATER_PIN, 0); // Catalytic Heating OFF
  baffleServo.write(0);              // Tesla Valve Baffle Fully Open (Zero resistance)

  // LCD Boot & Preheat
  lcd.init();
  lcd.backlight();
  lcd.print("System Starting");
  
  preheatSensors();
  lcd.clear();
}


void loop() {
  // 1. Read Environmental Data
  float pm25 = readPM25();
  float co = readCO();
  int co2 = readCO2();

  // Patent Closed-Loop Control Algorithm (Differential Response, Temp Control & Tesla Valve Flow Control)
  patentAirQualityControl(pm25, co);

  // UI Display & Serial Logging
  displayData(pm25, co, co2);
  logSensorData(pm25, co, co2);

  delay(2000);  // Sampling Interval 2000ms
}


//  Core Closed-Loop Control Algorithm 
void patentAirQualityControl(float pm25, float co) {
  bool needFan = false;
  bool needPump = false;
  bool needAlarm = false;
  bool needSlowDown = false; 
  int heaterPwm = 0; // PWM variable for catalytic heating (0-255)

  // CO Pollution Response (Coordinate Pump & Catalytic Heating)
  if (co > CO_SAFE_MAX) {
    needFan = true;
    needPump = true;    
    needAlarm = true;
    needSlowDown = true;     // Severe pollution: expand Tesla Valve to reduce flow rate for thorough catalysis
    heaterPwm = 255;         // 100% full power catalytic heating
    alarmEndTime = millis() + 30000;
  }
  else if (co > CO_GOOD_MAX) {
    needFan = true;
    needPump = true; 
    heaterPwm = 128;         // 50% power for insulation heating, energy saving for mild pollution
  }

  // PM2.5 & SO2 Pollution Response (Alkaline Adsorption)
  if (pm25 > PM25_SAFE_MAX) {
    needFan = true;
    needAlarm = true;
    needSlowDown = true;     // Severe smog: use Tesla Valve vortex resistance to increase filter contact time
    alarmEndTime = millis() + 30000;
  }
  else if (pm25 > PM25_GOOD_MAX) {
    needFan = true;
    // For mild dust pollution only: Fan ON, Pump/Heating OFF. Pure physical/chemical adsorption to save energy.
  }

  // Delay Safety State & Energy Saving Reset Check
  bool stillAlarm = (millis() < alarmEndTime) || needAlarm;

  if (!stillAlarm && pm25 <= PM25_GOOD_MAX && co <= CO_GOOD_MAX) {
    // Pollution completely cleared and 30s exhaust period ended -> Deep Energy Saving Mode
    needFan = false;        
    needPump = false;       
    heaterPwm = 0;          
    needSlowDown = false;   
  } else {
    // Keep fan running to exhaust residual gases if still in alarm delay
    needFan = true;
  }

  // Execute Peripheral Electromechanical Output
  digitalWrite(FAN_RELAY_PIN, needFan ? HIGH : LOW);
  digitalWrite(PUMP_RELAY_PIN, needPump ? HIGH : LOW);
  analogWrite(CATALYTIC_HEATER_PIN, heaterPwm); // Dynamic PWM temp control

  digitalWrite(LED_PIN, stillAlarm ? HIGH : LOW);
  digitalWrite(BUZZER_PIN, stillAlarm ? LOW : HIGH); 

  // Tesla Valve Dynamic Q=v*A Resistance & State Lock Logic (Motor Protection)
  if (needSlowDown && !isTeslaExpanded) {
    // Action: Needs deceleration and currently not expanded -> Execute Expand
    digitalWrite(ACTUATOR_IN1, HIGH);
    digitalWrite(ACTUATOR_IN2, LOW);
    baffleServo.write(45);           // Rotate servo to establish reverse flow resistance
    delay(300);                      // Wait for actuator stroke
    digitalWrite(ACTUATOR_IN1, LOW); // Cut power to lock motor
    isTeslaExpanded = true;          // Update state lock
  } 
  else if (!needSlowDown && isTeslaExpanded) {
    // Action: Safety restored and currently expanded -> Execute Retract/Reset
    digitalWrite(ACTUATOR_IN1, LOW);
    digitalWrite(ACTUATOR_IN2, HIGH);
    baffleServo.write(0);            // Reset baffle to zero
    delay(300);                      // Wait for actuator to retract
    digitalWrite(ACTUATOR_IN2, LOW); // Cut power to lock motor
    isTeslaExpanded = false;         // Update state lock
  }
}


//  Sensor Reading & Utility Functions 

void preheatSensors() {
  // MQ-7 Preheat
  lcd.setCursor(0, 1);
  lcd.print("Preheating CH4...");
  for (int i = 0; i < 30; i++) {
    delay(1000);
    if (i % 5 == 0) lcd.print(".");
  }
  
  // CO2 Preheat Countdown
  lcd.clear();
  lcd.print("CO2 Preheating");
  for (int i = 30; i < 300; i++) {
    delay(1000);
    if (i % 10 == 0) {
      lcd.setCursor(0, 1);
      int remain = 300 - i;
      lcd.print(remain / 60);
      lcd.print("m ");
      lcd.print(remain % 60);
      lcd.print("s   ");
    }
  }
}

void displayData(float pm25, float co, int co2) {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("PM:");
  lcd.print((int)pm25);
  if (pm25 > PM25_SAFE_MAX) {
    lcd.print("!!");  
  } 
  else if (pm25 > PM25_GOOD_MAX) {
    lcd.print("!");   
  }

  lcd.setCursor(0, 1);
  lcd.print("CO:");
  lcd.print((int)co);
  if (co > CO_SAFE_MAX) {
    lcd.print("!!");  
  } 
  else if (co > CO_GOOD_MAX) {
    lcd.print("!");   
  }

  lcd.print(" CO2:");
  if (co2 >= 0) {
    lcd.print(co2);
  } 
  else {
    lcd.print("ERR"); 
  }
}

void logSensorData(float pm25, float co, int co2) {
  Serial.print("PM2.5: ");
  Serial.print(pm25);
  Serial.print(" ug/m3 | ");
  
  Serial.print("CO: ");
  Serial.print(co);
  Serial.print(" ppm | ");
  
  Serial.print("CO2: ");
  if (co2 >= 0) {
    Serial.print(co2);
  } 
  else {
    Serial.print("ERROR");
  }
  Serial.println(" ppm");
}

float readPM25() {
  digitalWrite(PM_LED_PIN, LOW);
  delayMicroseconds(280);
  int val = analogRead(PM_PIN);
  delayMicroseconds(40);
  digitalWrite(PM_LED_PIN, HIGH);
  delayMicroseconds(9680);
  
  float voltage = val * (5.0 / 1024.0);
  float density = (0.17 * voltage - 0.1) * 1000;
  return max(0.0, density);  
}

float readCO() {
  int val = analogRead(MQ7_PIN);
  float rs = RL_VALUE * ((1023.0 / val) - 1);
  return 10.0 * pow(rs / RO_CLEAN_AIR, -2.0);
}

int readCO2() {
  // Send read command
  byte cmd[6] = {0x2C, 0x01, 0x00, 0x00, 0x00, 0x2D};
  co2Serial.write(cmd, 6);
  
  delay(100);
  
  if (co2Serial.available() >= 6) {
    byte buffer[6];
    for (int i = 0; i < 6; i++) {
      buffer[i] = co2Serial.read();
    }
    
    // Checksum verification
    byte checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4];
    if (checksum == buffer[5]) {
      return constrain((buffer[1] << 8) | buffer[2], 350, 2000);
    }
  }
  return -1; 
}
