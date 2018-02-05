#include "SevSeg.h"
SevSeg sevseg;

int box = 2,
    button = 3,
    poti = A0,
    runden = 0,
    potivalue = 0,
    buttonstate = 0,
    lastbuttonstate = 0,
    tmp = 0,
    i = 0,
    j = 0,
    k = 0;

unsigned long runde = 0,
              pause = 0,
              zeit = 0;
//
int getPotiValue();
void beep(int a, int b, int anzahl);
void wasteTime(unsigned long t, int j);

void setup() {
  byte numDigits = 1;
  byte digitPins[] = {12}; //Digits: 1,2,3,4 <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {4 , 5 , 6 , 7, 8, 9, 10 , 11 }; //Segments: A,B,C,D,E,F,G,Period

  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(100);

  pinMode(box, OUTPUT);
  pinMode(button, INPUT);
}

void loop() {
  runde = 0;
  pause = 0;
  // lese Rundendauer ein
  runde = 60000 * getPotiValue();
  // lese Pausendauer ein
  pause = 60000 * getPotiValue();
  // lese Rundenanzahl ein
  runden = getPotiValue();
  // Durchlauf von Runde+Pause
  for (k = 1; k <= runden; k++) {
    // Beep für start der Runde
    beep(500, 250, 4);
    // Runde verstreichen lassen
    wasteTime(Runde, k);
    // Beep Start der Pause
    beep(100, 250, 5);
    // Pause verstreichen lassen, wenn nicht letzte runde
    if (k < runden) wasteTime(pause, k);
  }
}

void beep(int a, int b, int anzahl) {
  // la=länge v Beep, lb=Pause zwischen Beep, anzahl=Anzahl der Beeps
  int la = a/6;
  int lb = b/6;
  
  sevseg.setNumber(0, 0);
  
  for (i = 1; i <= anzahl; i++) {  
    
    digitalWrite(box, HIGH);
    for (j = 4; j <= 9; j++) {
      digitalWrite(j, LOW);
      delay(la);
    }
    
    digitalWrite(box, LOW);
    for (j = 9; j >= 4; j--) {
      digitalWrite(j, HIGH);
      delay(lb);
    }
  }
}

void wasteTime(unsigned long t ,int k) {
  zeit = millis();
  // Bleibe in der Schleife bis Pause verstrichen ist
  while (millis() - zeit < t) {
    sevseg.setNumber(k, 0);
    sevseg.refreshDisplay();
  }
}

int getPotiValue() {
  tmp = 0;
  buttonstate = 0;
  lastbuttonstate = 0;

  while (true) {
    sevseg.setNumber(tmp);
    sevseg.refreshDisplay();
    buttonstate = digitalRead(button);

    // Button press
    if (buttonstate != lastbuttonstate && buttonstate == HIGH) {
      zeit = millis();
      digitalWrite(box,HIGH);
      delay(20);
      digitalWrite(box,LOW);
    }

    // Wenn button 2 sekunden gedrückt, dann return potivalue
    if (buttonstate == lastbuttonstate && buttonstate == HIGH) {
      if (millis() - zeit > 1000) {
        digitalWrite(box,HIGH);
        delay(40);
        digitalWrite(box,LOW);
        return tmp;
      }
    }
    
    lastbuttonstate = buttonstate;
    potivalue = analogRead(poti);
    tmp = map(potivalue,0,1027,0,10);
  }
}
