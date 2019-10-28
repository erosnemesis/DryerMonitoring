PRODUCT_ID(10308);

PRODUCT_VERSION(1);
/*
 * Project Nhinja Monitoring
 * Description: Dryer Monitoring IOT project for telecommunication air dryers
 * Author: Robert Bachta @ PacTel Solutions
 * Date: 2019
 */

#include "CurrentMonitor.h"

#define MONITOR_DEBUG false

const uint8_t relayCount = 4;
const uint8_t alarmCount = 4;
const uint8_t ampCount = 8;
unsigned long debounceTime = millis();
const uint8_t DEBOUNCE_DELAY = 200;
const uint16_t RELAY_DELAY = 500;
const uint16_t knownVoltage = 118;
String signalStrength = "0";
String signalQuality = "0";

const String PUBLISH_NAME = "Dryer_Alarms"; // DO NOT MODIFY THIS. GOOGLE CLOUD PUBSUB DEPENDS ON THIS
const uint16_t rPins[relayCount] = {D3, D2, D1, D0};
const uint8_t ALARM[alarmCount] = {D4, D5, D6, D8};
int alarmValues[alarmCount] = {0, 0, 0, 0};
byte alarmState[alarmCount] = {0, 0, 0, 0};
String ampValue[ampCount];
String powerValue[ampCount];
String names[alarmCount];

//current adc chip
CurrentMonitor monitor;

// function declarations
void setAlarm(bool inAlarm, int alarmNum);
int alarmReset(String alarmNum);
void setAmpReadings();

// setup() runs once, when the device is first turned on.
void setup() {

  #if MONITOR_DEBUG
    Serial.begin();
  #endif

  for(int i = 0; i < alarmCount; i++){
    pinMode(ALARM[i], INPUT);
    names[i] = "Dryer_Alarm_" + String(i+1);
  }

  Particle.function("Reset_Dryer", alarmReset);

  for(int i = 0; i < relayCount; i++){
    pinMode(rPins[i], OUTPUT);
    digitalWrite(rPins[i], HIGH);
  }

  monitor.begin();

  Particle.variable("Signal_Strength", signalStrength);
  Particle.variable("Signal_Quality", signalQuality);
  for(uint8_t i = 0; i < ampCount; i++){
    Particle.variable("Amp_" + String(i), ampValue[i]);
    Particle.variable("Power_" + String(i), powerValue[i]);
  }

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  for(int i = 0; i < alarmCount; i++){
    alarmValues[i] = digitalRead(ALARM[i]);
    alarmValues[i] == HIGH ? setAlarm(true, i) : setAlarm(false, i);
    
    #if MONITOR_DEBUG
    Serial.print(String(i) + " Value: " + String(alarmValues[i]));
    Serial.print(" ");
    #endif
  }
  #if MONITOR_DEBUG
  Serial.println();
  #endif

  CellularSignal sig = Cellular.RSSI();
  signalStrength = String(sig.getStrength());
  signalQuality = String(sig.getQuality());

  setAmpReadings();
  #if MONITOR_DEBUG
  for(int i = 0; i < ampCount; i++){
    Serial.print("Irms ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(ampValue[i]);
  }
  Serial.println();
  for(int i = 0; i < ampCount; i++){
    Serial.print("Power ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(powerValue[i]);
  }
  Serial.println();
  #endif
}

void setAmpReadings(){
  for(uint8_t i = 0; i < ampCount; i++){
    double amp, power;
    amp = monitor.processAdc(i);
    power = amp * knownVoltage;
    ampValue[i] = String(amp);
    powerValue[i] = String(power);
  }
}

void setAlarm(bool inAlarm, int alarmNum){
  
  if(inAlarm){
    String alarmStr = String("Dryer ") + String(alarmNum+1) + String(" in Alarm");

    if(((millis() - debounceTime) > DEBOUNCE_DELAY) && alarmState[alarmNum] == 0){
      Particle.publish(PUBLISH_NAME, alarmStr, PRIVATE);
      debounceTime = millis();
      #if MONITOR_DEBUG
      Serial.println(alarmStr);
      #endif
    }
    alarmState[alarmNum] = 1;
    return;
  }

  if(((millis() - debounceTime) > DEBOUNCE_DELAY) && alarmState[alarmNum] == 1){
    String resetStr = String("Dryer Alarm " + String(alarmNum+1) + " Reset");
    Particle.publish(PUBLISH_NAME, resetStr, PRIVATE);
    debounceTime = millis();
    #if MONITOR_DEBUG
    Serial.println(resetStr);
    #endif
  }
  alarmState[alarmNum] = 0;
}

int alarmReset(String alarmNum){
  int alarm = alarmNum.toInt();
  if(alarm < 1 || alarm > relayCount){
    return 0;
  }

  digitalWrite(rPins[alarm-1], LOW);
  long resetDelay = millis();
  while(millis() - resetDelay < RELAY_DELAY){
    //wait (this is here instead to delay() so interrupts can occur)
  }
  digitalWrite(rPins[alarm-1], HIGH);

  Particle.publish(PUBLISH_NAME, String("Remote Dryer Alarm " + alarmNum + " Reset Sent"), PRIVATE);

  #if MONITOR_DEBUG
  Serial.println("Remote Dryer Alarm " + alarmNum + " Reset Sent");
  #endif
  return 1;
}