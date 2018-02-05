#include "SevSeg.h"
SevSeg sevseg;

int box = 2;
int button = 3;
int poti = A0;

int runden = 0;
int potivalue = 0;
int buttonstate = 0;
int lastbuttonstate = 0;

int tmp = 0;
int i = 0;
int j = 0;
int k = 0;

unsigned long Runde = 0;
unsigned long Pause = 0;
unsigned long zeit = 0;

int GetPotiValue();
void Beep(int a, int b, int anzahl);
void WasteTime(unsigned long t, int j);

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
  Runde = 0;
  Pause = 0;
  // lese Rundendauer ein
  Runde = 60000 * GetPotiValue();
  // lese Pausendauer ein
  Pause = 60000 * GetPotiValue();
  // lese Rundenanzahl ein
  runden = GetPotiValue();
  // Durchlauf von Runde+Pause
  for (k = 1; k <= runden; k++) {
    // Beep für start der Runde
    Beep(500, 250, 4);
    // Runde verstreichen lassen
    WasteTime(Runde, k);
    // Beep Start der Pause
    Beep(100, 250, 5);
    // Pause verstreichen lassen, wenn nicht letzte runde
    if (k < runden) WasteTime(Pause, k);
  }
}

void Beep(int a, int b, int anzahl) {
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

void WasteTime(unsigned long t ,int k) {
  zeit = millis();
  // Bleibe in der Schleife bis Pause verstrichen ist
  while (millis() - zeit < t) {
    sevseg.setNumber(k, 0);
    sevseg.refreshDisplay();
  }
}

int GetPotiValue() {
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
    potivalue=analogRead(poti);
    tmp=map(potivalue,0,1027,0,10);
  }
}
