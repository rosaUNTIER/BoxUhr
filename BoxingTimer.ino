#include "SevSeg.h"
SevSeg sevseg;

int LEDr = A3;
int LEDy = A4;
int LEDg = A5;
int box = A0;
int button = A1;
int poti = A2;

int runden = 0;
int potivalue = 0;
int buttonstate = 0;
int lastbuttonstate = 0;
int LEDstate = 0;
int lastLEDstate = HIGH;

int menu= 0;

int tmp = 0;
int tmp3 = 0;
int i = 0;
int j = 0;
int k = 0;

unsigned long Runde = 0;
unsigned long Pause = 0;
unsigned long zeit = 0;

int mins=0;
int secs=0;
unsigned long sekunde = 1000;
unsigned long tmp1=0;
unsigned long tmp2=0;

int mmax = 2;
int smax = 59;
int rmax = 15;
unsigned long Rundemultiplier =60000;
unsigned long Pausemultiplier =1000;

int GetPotiValue(char selection);
void WaitForPress();
void Beep(int a, int b, int anzahl);
void WasteTime(unsigned long t, int j);

void setup() {
  byte numDigits = 4;
  byte digitPins[] = {2, 5, 6, 13}; //Digits: 1,2,3,4
  byte segmentPins[] = {3, 7, 11, 9, 8, 4, 12, 10}; //Segments: A,B,C,D,E,F,G,Period

  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(100);

  pinMode(box, OUTPUT);
  pinMode(button, INPUT);
  pinMode(LEDr, OUTPUT);
  pinMode(LEDy, OUTPUT);
  pinMode(LEDg, OUTPUT);
}

void loop() {
  Runde = 0;
  Pause = 0;
  
  // Warte auf Start von Einstellungen
  digitalWrite(LEDr, HIGH);
  digitalWrite(LEDy, HIGH);
  digitalWrite(LEDg, HIGH);
  WaitForPress();
  
  // lese Rundendauer ein
  digitalWrite(LEDr, LOW);
  digitalWrite(LEDy, LOW);
  digitalWrite(LEDg, HIGH);
  Runde = Rundemultiplier * GetPotiValue(0);
  
  // lese Pausendauer ein
  digitalWrite(LEDr, LOW);
  digitalWrite(LEDy, HIGH);
  digitalWrite(LEDg, LOW);
  Pause = Pausemultiplier * GetPotiValue(1);
  
  // lese Rundenanzahl ein
  digitalWrite(LEDr, HIGH);
  digitalWrite(LEDy, LOW);
  digitalWrite(LEDg, LOW);
  runden = GetPotiValue(2);
  
  // Durchlauf von Runde+Pause
  for (k = 1; k <=runden; k++) {
    
    // Beep für start der Runde
    Beep(500, 250, 4);
    
    // Runde verstreichen lassen
    digitalWrite(LEDr, LOW);
    digitalWrite(LEDy, LOW);
    digitalWrite(LEDg, HIGH);
    WasteTime(Runde, k);
    
    // Beep Start der Pause
    Beep(100, 250, 5);
    
    // Pause verstreichen lassen, wenn nicht letzte runde
    digitalWrite(LEDr, HIGH);
    digitalWrite(LEDy, LOW);
    digitalWrite(LEDg, LOW);
    if (k < runden) WasteTime(Pause, k);
  }
}

void WaitForPress(){
  buttonstate = 0;
  lastbuttonstate = 0;
  while(true){
    buttonstate = digitalRead(button);
    
    // Button press
    if (buttonstate != lastbuttonstate && buttonstate == HIGH) {
      zeit = millis();
      digitalWrite(box, HIGH);
      delay(20);
      digitalWrite(box, LOW);
    }

    // Wenn button 600ms gedrückt, dann break
    if (buttonstate == lastbuttonstate && buttonstate == HIGH) {
      if (millis() - zeit > 600) {
        digitalWrite(box, HIGH);
        delay(30);
        digitalWrite(box, LOW);
        break;
      }
    }
    
    lastbuttonstate = buttonstate;
  }
}
void Beep(int a, int b, int anzahl) {
  // a=länge v beep, b=pause zw beep, anzahl=Anzahl der Beeps
  
  for (i = 1; i <= anzahl; i++) {    
    digitalWrite(box, HIGH);
    digitalWrite(LEDr, HIGH);
    digitalWrite(LEDy, HIGH);
    digitalWrite(LEDg, HIGH);
    delay(a);
    digitalWrite(box, LOW);
    digitalWrite(LEDr, LOW);
    digitalWrite(LEDy, LOW);
    digitalWrite(LEDg, LOW);
    delay(b);
  }
}

void WasteTime(unsigned long t ,int k) {
  zeit = millis();
  mins=t/60000;
  secs=0;
  sekunde = 1000;
  tmp1=0;
  tmp2=0;

  // Bleibe in der Schleife bis Pause verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (millis() - zeit < t) {
    tmp1 = millis() - zeit;
    
    if(tmp1 % sekunde == 0 && tmp1 != tmp2){
      secs = secs - 1;
      sevseg.setNumber(100 * mins + secs, 2);
      tmp2 = tmp1;
    }
    if(secs == 0){
      mins = mins - 1;
      secs = 60;
    }
    if(mins == 0 && secs == 30){
      digitalWrite(LEDr, LOW);
      digitalWrite(LEDy, HIGH);
      digitalWrite(LEDg, LOW);
    }
    sevseg.refreshDisplay();
  }
}

int GetPotiValue(int selection) {
  tmp = 0;
  tmp1 = millis();
  tmp2 = 0;
  tmp3 = 0;
  buttonstate = 0;
  lastbuttonstate = 0;
  
  static int bound = 0;
  static int blinkLED = 0;

  // Maximum für Potivalue <-- mmax(Minunten), smax(Sekunden), rmax(Runden)
  // blinkLED <-- LEDr, LEDy, LEDg
  switch(selection){
  case 0 :
    bound = mmax;
    blinkLED = LEDg;
    tmp = 3;
    break;
  case 1 :
    bound = smax;
    blinkLED = LEDy;
    tmp = 60;
    break;
  case 2 :
    bound = rmax;
    blinkLED = LEDr;
    tmp = 15;
    break;
  }

  while (true) {
    sevseg.setNumber(tmp);
    sevseg.refreshDisplay();
    buttonstate = digitalRead(button);
    tmp2 = millis();

    // blinkLED blinkt
    if((tmp2 - tmp1) >= 250){
      if(tmp3 % 2 == 0 && LEDstate == LOW){
        digitalWrite(blinkLED, HIGH);
        LEDstate = HIGH;
        tmp1 = millis();
      }
      else if(tmp3 % 2 == 1 && LEDstate == HIGH){
        digitalWrite(blinkLED, LOW);
        LEDstate = LOW;
        tmp1 = millis();
      }
        tmp3 = tmp3 + 1;              
    }
    
    // Button press
    if (buttonstate != lastbuttonstate && buttonstate == HIGH) {
      zeit = millis();
      digitalWrite(box, HIGH);
      delay(20);
      digitalWrite(box, LOW);
    }

    // Wenn button 600ms gedrückt, dann return potivalue
    if (buttonstate == lastbuttonstate && buttonstate == HIGH) {
      if (millis() - zeit > 600) {
        digitalWrite(box, HIGH);
        delay(30);
        digitalWrite(box, LOW);
        return tmp;
      }
    }
    
    lastbuttonstate = buttonstate;
    potivalue = analogRead(poti);
    tmp = map(potivalue, 0, 1023, 1, bound+1);
  }
}
