/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "c:/Users/erosn/ownCloud/ParticleProjects/Monitoring/src/Monitoring.ino"
/*
 * Project Monitoring
 * Description:
 * Author:
 * Date:
 */

#include "CurrentMonitor.h"

void setup();
void loop();
#line 10 "c:/Users/erosn/ownCloud/ParticleProjects/Monitoring/src/Monitoring.ino"
#define MONITOR_DEBUG true

const uint8_t relayCount = 4;
const uint8_t alarmCount = 4;
const uint16_t rPins[relayCount] = {D3, D2, D1, D0};
unsigned long debounceTime = millis();
unsigned const int DEBOUNCE_DELAY = 200;
String signalStrength = "0";
String signalQuality = "0";
String power = "0";
String Amps = "0";

const uint8_t ALARM[alarmCount] = {D4, D5, D6, D8};
int alarmValues[alarmCount] = {0, 0, 0, 0};
byte alarmState[alarmCount] = {0, 0, 0, 0};
String names[alarmCount];

//current adc chip
CurrentMonitor monitor;

// function declarations
void setAlarm(bool inAlarm, int alarmNum);
int alarmReset(String alarmNum);

// setup() runs once, when the device is first turned on.
void setup() {

  if(MONITOR_DEBUG){
    Serial.begin();
  }

  for(int i = 0; i < alarmCount; i++){
    pinMode(ALARM[i], INPUT);
    names[i] = "Dryer_Alarm_" + String(i+1);
  }

  Particle.function("Reset_Dryer", alarmReset);

  for(int i = 0; i < relayCount; i++){
    pinMode(rPins[i], OUTPUT);
    digitalWrite(rPins[i], HIGH);
  }

  //adc.begin(SCK, MOSI, MISO, SS);
  monitor.begin();

  Particle.variable("Signal_Strength", signalStrength);
  Particle.variable("Signal_Quality", signalQuality);
  Particle.variable("Power", power);
  Particle.variable("Amps", Amps);
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

  Serial.print("Irms 0: ");
  double Irms = monitor.processAdc(0);
  Amps = String(Irms);
  power = String(Irms * 118);
  Serial.print(Irms * 118);
  Serial.print(" ");
  Serial.println(Irms);
}

void setAlarm(bool inAlarm, int alarmNum){
  String alarmStr = "In Alarm";
  if(inAlarm){
    //strip.setPixelColor(alarmNum, BRIGHTNESS, 0, 0);
    if(((millis() - debounceTime) > DEBOUNCE_DELAY) && alarmState[alarmNum] == 0){
      Particle.publish(names[alarmNum], alarmStr, 259200, PRIVATE);
      debounceTime = millis();
    }
    alarmState[alarmNum] = 1;
    return;
  }

  //strip.setPixelColor(alarmNum, 0, 0, 0);
  if(((millis() - debounceTime) > DEBOUNCE_DELAY) && alarmState[alarmNum] == 1){
    Particle.publish(names[alarmNum], String("Alarm Reset"), 259200, PRIVATE);
    debounceTime = millis();
  }
  alarmState[alarmNum] = 0;
}

int alarmReset(String alarmNum){
  int alarm = alarmNum.toInt();
  if(alarm < 1 || alarm > relayCount){
    return 0;
  }
  digitalWrite(rPins[alarm-1], LOW);
  delay(500);
  digitalWrite(rPins[alarm-1], HIGH);
  Particle.publish(names[alarm-1], String("Alarm Reset"), 259200, PRIVATE);
  return 1;
}