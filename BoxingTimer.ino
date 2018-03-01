#include "SevSeg.h"
SevSeg sevseg;

int LEDr = A3; // 1k in Serie gegen Ground
int LEDy = A4; // 1k in Serie gegen Ground
int LEDg = A5; // 1k in Serie gegen Ground
int box = A0; // gegen Ground
int button = A1; // 330 in Serie gegen Ground
int poti = A2; // Stützkondensator parallel

// box Variablen
int boxState = 0;
unsigned long tmpBox1 = 0;
unsigned long tmpBox2 = 0;

// Segment Variablen
int rundenAnzahl = 2;
int tRunde = 300;
int tPause = 100;

// CountToZero Variablen
int timeLeft = 0;

const unsigned long sekunde = 1000;
unsigned long tmpSekunde1 = 0;
unsigned long tmpSekunde2 = 0;

// blink LED Variablen
int blinkLED = 0;
boolean LEDState = false;
int blinkCounter = 0;
unsigned long tmpBlink1 = 0;
unsigned long tmpBlink2 = 0;

// Button Variablen
int buttonState = 0;
int lastButtonState = 0;
unsigned long timeStamp = 0;
unsigned long timeStampMenu = 0;

// GetPotiValue Variablen
int lastPotiValue = 9999;
int potiValue = 0;
int tmp = 0;
const int tMax = 300;
const int rMax = 16;

// Funktionen
int getPotiValue(int selection);
void beep(unsigned long a, unsigned long b, int anzahl);
void countToZero(int Time, int selection);
void setLEDs(int setLEDg, int setLEDy, int setLEDr);
void segment(int tRunde, int tPause, int rundenAnzahl);
void hold();

void setup() {

  // Definiere 7-Segment Display
  byte numDigits = 4;
  byte digitPins[] = {2, 5, 6, 13}; //Digits: 1,2,3,4
  byte segmentPins[] = {3, 7, 11, 9, 8, 4, 12, 10}; //Segments: A,B,C,D,E,F,G,Period
  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins, false, false, true);
  sevseg.setBrightness(100);

  // Setze Pins
  pinMode(box, OUTPUT);
  pinMode(button, INPUT);
  pinMode(LEDr, OUTPUT);
  pinMode(LEDy, OUTPUT);
  pinMode(LEDg, OUTPUT);
}

void loop() {
  
  // Button press
  buttonState = digitalRead(button);
  if (buttonState != lastButtonState && buttonState == HIGH) {
    beep(20, 0, 1);
    // double-Click --> Parameter Name auf 7Seg
    if (millis() - timeStamp < 400) timeStampMenu = millis();
    timeStamp = millis();
  }
  
  // Wenn button 600ms gedrückt, dann
  // Lese Werte ein Grün = Rundendauer
  // Rot = Pausendauer, Gelb = Anzahl an Runden
  // Alle = Start
  if (buttonState == lastButtonState && buttonState == HIGH) {
    if (millis() - timeStamp > 600) {
      switch (potiValue){
        case 1 :
          beep(30, 0, 1);
          tRunde = getPotiValue(0);
          timeStamp = millis();
        break;
        case 2 :
          beep(30, 0, 1);
          rundenAnzahl = getPotiValue(2);
          timeStamp = millis();
        break;
        case 3 :
          beep(30, 0, 1);
          tPause = getPotiValue(1);
          timeStamp = millis();
        break;
        case 4 :
          beep(30, 0, 1);
          segment(tRunde, tPause, rundenAnzahl);
          timeStamp = millis();
        break;
      }
    }
  }
  lastButtonState = buttonState;
  
  // display PotiValue on LEDs and
  // tRunde/tPause/rundenAnzahl/Gesamtzeit on 7-Seg
  potiValue = map(analogRead(poti), 0, 1023, 1, 4);
  
  if (potiValue != lastPotiValue){
    timeStampMenu = millis();
  }

  lastPotiValue = potiValue;

  // je nach Potivalue --> LED leuchtet, parameter name auf 7Seg für 500ms danach parameter Wert auf 7Seg
  switch (potiValue){
    case 1 :
      setLEDs(HIGH, LOW, LOW);
      if (millis() - timeStampMenu < 500){
        sevseg.setChars("rnde");
      }
      else sevseg.setNumber(tRunde, 2);
    break;
    case 2 :
      setLEDs(LOW, HIGH, LOW);
      if (millis() - timeStampMenu < 500){
        sevseg.setChars("rndn");
      }
      else sevseg.setNumber(rundenAnzahl, 0);
    break;
    case 3 :
      setLEDs(LOW, LOW, HIGH);
      if (millis() - timeStampMenu < 500){
        sevseg.setChars("paus");
      }
      else sevseg.setNumber(tPause, 2);
    break;
    case 4 :
      // Blink
      tmpBlink1 = millis();
    
      if ((tmpBlink1 - tmpBlink2) >= 250){
        
        tmpBlink2 = millis();
        LEDState = !LEDState;
        setLEDs(LEDState, LEDState, LEDState);         
      }
      if (millis() - timeStampMenu < 500){
        sevseg.setChars(" go ");
      }
      else{
        // tmp = Gesamtlaufzeit
        tmp = 100*((tRunde/100)*rundenAnzahl) + 100*(((tRunde%100)*rundenAnzahl)/60) + ((tRunde%100)*rundenAnzahl)%60;
        if(rundenAnzahl > 1){
          tmp += 100*((tPause/100)*(rundenAnzahl-1)) + 100*(((tPause%100)*(rundenAnzahl-1))/60) + ((tPause%100)*(rundenAnzahl-1))%60;
        }
        sevseg.setNumber(tmp, 2); 
      }   
      break;
    }
  
  sevseg.refreshDisplay();
}

void segment(int tRunde, int tPause, int rundenAnzahl){
  static int k = 0;
  static int T = 0;
  
  T = rundenAnzahl;
  
  for (k = 1; k <= T; k++) {
    
    // Beep für start der Runde
    beep(500, 200, 5);
    
    // Runde verstreichen lassen
    setLEDs(HIGH, LOW, LOW);
    countToZero(tRunde, 0);
    
    // Beep Start der Pause
    beep(100, 250, 5);
    
    // Pause verstreichen lassen
    // wenn nicht letzte runde
    if (k < T){
      setLEDs(LOW, LOW, HIGH);
      countToZero(tPause, 1);
    }
  }
}

void setLEDs(int setLEDg, int setLEDy, int setLEDr){
  digitalWrite(LEDr, setLEDr);
  digitalWrite(LEDg, setLEDg);
  digitalWrite(LEDy, setLEDy);
}

void beep(unsigned long a, unsigned long b, int anzahl) {
  // a=länge v beep, b=pause zw beep, anzahl=Anzahl der Beeps
  static int i = 0;
  i = anzahl;
  tmpBox1 = 0;
  tmpBox2 = millis();

  digitalWrite(box, HIGH);
  boxState = HIGH;
  sevseg.setChars("beep");
  
  while(true){
    sevseg.refreshDisplay();
    tmpBox1 = millis();

    if(i == 0){
      break;
    } 
    else if((tmpBox1 - tmpBox2) >= b && boxState == LOW){
      sevseg.setChars("beep");
      tmpBox2 = millis();
      digitalWrite(box, HIGH);
      boxState = HIGH;
    }
    else if((tmpBox1 - tmpBox2) >=a && boxState == HIGH){
      sevseg.setChars("");
      tmpBox2 = millis();
      digitalWrite(box, LOW);
      boxState = LOW;
      i -= 1;            
    }          
  }
}

void countToZero(int Time, int selection) {
  tmpBlink1 = 0;
  tmpBlink2 = 0;
  blinkCounter = 0;
  LEDState = LOW;
  
  timeLeft = Time;
  tmpSekunde1=0;
  tmpSekunde2=0;

  // Bleibe in der Schleife bis Time verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (true) {
      
    // Button press
    buttonState = digitalRead(button);
    if (buttonState != lastButtonState && buttonState == HIGH) {
      timeStamp = millis();
      beep(20, 0, 1);
    }

    // Wenn button 600ms gedrückt, dann hold()
    if (buttonState == lastButtonState && buttonState == HIGH) {
      if (millis() - timeStamp > 600) {
        beep(30, 0, 1);
        hold();
        if (timeLeft <= 30) setLEDs(LOW, HIGH, LOW);
        else if (selection == 0) setLEDs(HIGH, LOW, LOW);
        else if (selection == 1) setLEDs(LOW, LOW, HIGH);
      }
    }
    lastButtonState = buttonState;
      
    tmpSekunde1 = millis();

    // Zähle Sekunden
    if(tmpSekunde1 - tmpSekunde2 >= sekunde){
      
      timeLeft -= 1;
      tmpSekunde2 = millis();

      // Ton Bei 10, 3, 2, 1 sekunden
      if(timeLeft == 10){
        beep(233, 100, 3);
      }
      if(timeLeft <= 3){
        beep(100, 0, 1);
      }

      // Überspringe _60 - _99
      if((timeLeft % 100) == 99){
        timeLeft -= 40;
      }

      // Ende wenn timeLeft = 0
      if(timeLeft == 0){
        break;
      }
    }

    // Gelbe LED leuchtet letzten 30 sekunden
    if(timeLeft <= 30){    
      // LED blinkt letzten 10 sekunden
      tmpBlink1 = millis();   
      if((tmpBlink1 - tmpBlink2) >= 333){    
        tmpBlink2 = millis();
        LEDState = !LEDState;
        setLEDs(LOW, LEDState, LOW);         
      }
    }
    // Display timeLeft
    sevseg.setNumber(timeLeft, 2);
    sevseg.refreshDisplay();
  }
}

int getPotiValue(int selection) {
  tmpBlink1 = 0;
  tmpBlink2 = 0;
  blinkCounter = 0;
  
  buttonState = 0;
  lastButtonState = 0;
  
  static int bound = 0;

  // Maximum für Potivalue <-- tMax(Zeit), rMax(Runden)
  // blinkLED <-- LEDr, LEDy, LEDg
  switch(selection){
  case 0 :
    bound = tMax;
    blinkLED = LEDg;
    break;
  case 1 :
    bound = tMax;
    blinkLED = LEDr;
    break;
  case 2 :
    bound = rMax;
    blinkLED = LEDy;
    break;
  }

  // Bleibe in Schleife bis Button gedrückt wird
  while (true) {
        
    // blinkLED blinkt
    tmpBlink1 = millis();
  
    if((tmpBlink1 - tmpBlink2) >= 250){
      
      tmpBlink2 = millis();
      LEDState = !LEDState;
      digitalWrite(blinkLED, LEDState);        
    }
    
    // Button press
    buttonState = digitalRead(button);
    if (buttonState != lastButtonState && buttonState == HIGH) {
      timeStamp = millis();
      beep(20, 0, 1);
    }

    // Wenn button 600ms gedrückt, dann return potiValue
    if (buttonState == lastButtonState && buttonState == HIGH) {
      if (millis() - timeStamp > 600) {
        beep(30, 0, 1);
        buttonState = LOW;
        lastButtonState = LOW;
        return tmp;
      }
    }
    lastButtonState = buttonState;

    // display PotiValue
    potiValue = map(analogRead(poti), 0, 1023, 1, bound);
    tmp = (potiValue / 60) * 100 + (potiValue % 60);   
    sevseg.setNumber(tmp, 2);
    sevseg.refreshDisplay();
  }
}

void hold(){

  buttonState = LOW;
  lastButtonState = LOW;

  tmpBlink1 = 0;
  tmpBlink2 = 0;
  LEDState = HIGH;
  
  while(true){

    // "halt" on 7Seg
    sevseg.setChars("halt");
    sevseg.refreshDisplay();

    // Blink
    tmpBlink1 = millis();
  
    if((tmpBlink1 - tmpBlink2) >= 250){
      
      tmpBlink2 = millis();
      LEDState = !LEDState;
      setLEDs(LEDState, LEDState, LEDState);         
    }
    
    // Button press
    buttonState = digitalRead(button);
    if (buttonState != lastButtonState && buttonState == HIGH) {
      timeStamp = millis();
      beep(20, 0, 1);
    }
    
    // Wenn button 600ms gedrückt, dann break
    if (buttonState == lastButtonState && buttonState == HIGH) {
      if (millis() - timeStamp > 600) {
        beep(30, 0, 1);
        buttonState = LOW;
        lastButtonState = LOW;
        break;
      }
    }
    lastButtonState = buttonState;
  }
}
