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
unsigned long tButton = 0;
int buttonState = 0;
int lastButtonState = 0;
unsigned long timeStampMenu = 0;

// GetPotiValue Variablen
int lastPotiValue = 9999;
int potiValue = 0;
int tmp = 0;
const int tMax = 300;
const int rMax = 16;

// StoppUhr Variablen
int timeCount = 55;
int timeFreeze = 0;
int *timeToDisplay = NULL;

// Funktionen
void main_Sets();
int getPotiValue(int selection);
void segment(int tRunde, int tPause, int rundenAnzahl);
void countToZero(int Time, int selection);
void hold();

void main_Stoppuhr();

void main_Game();

boolean buttonClick();
boolean doubleClick();
boolean longClick();

void setLEDs(int setLEDg, int setLEDy, int setLEDr);
void blinkLEDs(int selection);

void beep(unsigned long a, unsigned long b, int anzahl);

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
  potiValue = map(analogRead(poti), 0, 1023, 1, 3);
  switch(potiValue){
    case 1:
      sevseg.setChars("sets");
      setLEDs(HIGH, LOW, LOW);
      buttonClick();
      if (longClick() == true) main_Sets();
    break;
    case 2:
      sevseg.setChars("stop");
      setLEDs(LOW, HIGH, LOW);
      buttonClick();
      if (longClick() == true) main_Stoppuhr();
    break;
    case 3: 
      sevseg.setChars("rctn");
      setLEDs(LOW, LOW, HIGH);
      buttonClick();
      if (longClick() == true) main_Game();
    break;
  }
  sevseg.refreshDisplay();
}

void main_Sets() {
  timeStampMenu = millis();
  while (true){
    buttonClick();
    potiValue = map(analogRead(poti), 0, 1023, 0, 4);
    if (potiValue != lastPotiValue) timeStampMenu = millis();
    lastPotiValue = potiValue;
    
    switch (potiValue){
      case 0: 
        setLEDs(LOW, LOW, LOW);
        sevseg.setChars("back");
        if (longClick() == true) goto Exit_main_Sets;
      break;
      case 1: 
        setLEDs(HIGH, LOW, LOW);
        if (millis() - timeStampMenu < 500) sevseg.setChars("rnde");
        else sevseg.setNumber(tRunde, 2);
        if (longClick() == true) tRunde = getPotiValue(0);
      break;
      case 2:
        setLEDs(LOW, HIGH, LOW);      
        if (millis() - timeStampMenu < 500) sevseg.setChars("rndn");
        else sevseg.setNumber(rundenAnzahl);
        if (longClick() == true) rundenAnzahl = getPotiValue(2);
      break;
      case 3:
        setLEDs(LOW, LOW, HIGH);
        if (millis() - timeStampMenu < 500) sevseg.setChars("paus");
        else sevseg.setNumber(tPause, 2);
        if (longClick() == true) tPause = getPotiValue(1);
      break;
      case 4:
        blinkLEDs(4);
        if (millis() - timeStampMenu < 500) sevseg.setChars(" go ");
        else {
          tmp = 100*((tRunde/100)*rundenAnzahl) + 100*(((tRunde%100)*rundenAnzahl)/60) + ((tRunde%100)*rundenAnzahl)%60;
          if(rundenAnzahl > 1){
            tmp += 100*((tPause/100)*(rundenAnzahl-1)) + 100*(((tPause%100)*(rundenAnzahl-1))/60) + ((tPause%100)*(rundenAnzahl-1))%60;
          }
          sevseg.setNumber(tmp, 2); 
        }
        if (longClick() == true) segment(tRunde, tPause, rundenAnzahl);
      break;
    }
    sevseg.refreshDisplay();
  }
  Exit_main_Sets:;
}

void main_Stoppuhr() {
  while (true){
    potiValue = map(analogRead(poti), 0, 1023, 0, 1);
    buttonClick();
    switch (potiValue){
      case 0:
        sevseg.setChars("back");
        setLEDs(LOW, LOW, LOW);
        if (longClick() == true) goto Exit_Stoppuhr;
      break;
      case 1:
        sevseg.setChars(" go ");
        blinkLEDs(4);
        buttonClick();
        if (longClick() == true) goto Start_Stoppuhr;
      break;
    }
    sevseg.refreshDisplay();
  }
  Start_Stoppuhr:;
  setLEDs(HIGH, LOW, LOW);
  timeCount = -1;
  timeFreeze = 0;
  timeToDisplay = &timeCount;
  tmpSekunde1=0;
  tmpSekunde2=0;

  // Bleibe in der Schleife bis Time verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (true) {
    
    // Zähle Sekunden
    tmpSekunde1 = millis();
    if(tmpSekunde1 - tmpSekunde2 >= sekunde){
      
      timeCount += 1;
      tmpSekunde2 = millis();

      // Überspringe _60 - _99
      if ((timeCount % 100) == 60){
        timeCount += 40;
      }
    }
   
    // Button press
    if (longClick() == true){
      if (timeToDisplay == &timeCount){
        break;
      }
      else{
        setLEDs(LOW, LOW, HIGH);
        timeFreeze = timeCount;
        timeToDisplay = &timeFreeze;
        while (true){
          sevseg.setNumber(*timeToDisplay, 2);
          sevseg.refreshDisplay();
          if (buttonClick() == true) {
            timeToDisplay = &timeCount;
            setLEDs(HIGH, LOW, LOW);
            break;
          }
        }
      }
    }
    if (buttonClick() == true){
      if (timeToDisplay == &timeFreeze){
        timeToDisplay = &timeCount;
        setLEDs(HIGH, LOW, LOW);
      }
      else{
        timeFreeze = timeCount;
        timeToDisplay = &timeFreeze;
        setLEDs(LOW, HIGH, LOW);
      }
    }
    // Display timeToDisplay
    sevseg.setNumber(*timeToDisplay, 2);
    sevseg.refreshDisplay();
  }
  
Exit_Stoppuhr:;
}

void main_Game(){
  
}

void segment(int tRunde, int tPause, int rundenAnzahl) {
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

void setLEDs(int setLEDg, int setLEDy, int setLEDr) {
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
  timeLeft = Time;
  tmpSekunde1=0;
  tmpSekunde2=0;

  // Bleibe in der Schleife bis Time verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (true) {
      
    buttonClick();

    // Wenn button 600ms gedrückt, dann hold()
    if (longClick() == true) {
        hold();
        if (timeLeft <= 30) setLEDs(LOW, HIGH, LOW);
        else if (selection == 0) setLEDs(HIGH, LOW, LOW);
        else if (selection == 1) setLEDs(LOW, LOW, HIGH);
    }      
    tmpSekunde1 = millis();

    // Zähle Sekunden
    if(tmpSekunde1 - tmpSekunde2 >= sekunde){
      
      timeLeft -= 1;
      tmpSekunde2 = millis();

      // Ton Bei 10, 3, 2, 1 sekunden
      if(timeLeft == 10 && selection == 0){
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
    if(timeLeft <= 30 && selection == 0) blinkLEDs(5);
    else if(timeLeft <= 30 && selection == 1) blinkLEDs(7);
    
    // Display timeLeft
    sevseg.setNumber(timeLeft, 2);
    sevseg.refreshDisplay();
  }
}

int getPotiValue(int selection) {
   static int bound = 0;

  // Maximum für Potivalue <-- tMax(Zeit), rMax(Runden)
  // blinkLED <-- LEDr, LEDy, LEDg
  switch(selection){
  case 0 :
    bound = tMax;
    blinkLED = 1;
    break;
  case 1 :
    bound = tMax;
    blinkLED = 3;
    break;
  case 2 :
    bound = rMax;
    blinkLED = 2;
    break;
  }

  // Bleibe in Schleife bis Button gedrückt wird
  while (true) {
        
    blinkLEDs(blinkLED);    
    buttonClick();

    // Wenn button 600ms gedrückt, dann return potiValue
    if (longClick() == true) return tmp;
    
    // display PotiValue
    potiValue = map(analogRead(poti), 0, 1023, 1, bound);
    tmp = (potiValue / 60) * 100 + (potiValue % 60);   
    sevseg.setNumber(tmp, 2);
    sevseg.refreshDisplay();
  }
}

void hold() {
  
  while(true){

    // "halt" on 7Seg
    sevseg.setChars("halt");
    sevseg.refreshDisplay();

    // Blink
    blinkLEDs(4);
    
    buttonClick();
    
    // Wenn button 600ms gedrückt, dann break
    if (longClick() == true) break;
  }
}

boolean buttonClick() {
  
  buttonState = digitalRead(button);
  
  if (buttonState != lastButtonState && buttonState == HIGH && millis() - tButton > 400) {
   tButton = millis();
   beep(20, 0, 1);
   lastButtonState = buttonState;
   return true;
  }
  lastButtonState = buttonState;
  return false;
}

boolean doubleClick() {
  buttonState = digitalRead(button);
  if (buttonState != lastButtonState && buttonState == HIGH && millis() - tButton < 400) {
   beep(30, 0, 1);
   lastButtonState = buttonState;
   return true;
  }
  lastButtonState = buttonState;
  return false;
}

boolean longClick() {
   buttonState = digitalRead(button);
   if (buttonState == lastButtonState && buttonState == HIGH && millis() - tButton > 600) {
   beep(40, 0, 1);
   lastButtonState = LOW;
   tButton = millis();
   return true;
  }
  lastButtonState = lastButtonState;
  return false;
}

void blinkLEDs(int selection) {
  tmpBlink1 = millis();
    
  if ((tmpBlink1 - tmpBlink2) >= 250){      
    tmpBlink2 = millis();
    switch (selection){
      case 1: 
        setLEDs(LEDState, LOW, LOW);
      break;
      case 2:
        setLEDs(LOW, LEDState, LOW);
      break;
      case 3:
        setLEDs(LOW, LOW, LEDState);
      break;
      case 4:
        setLEDs(LEDState, LEDState, LEDState);
      break;
      case 5:
        setLEDs(LEDState, LEDState, LOW);
      break;
      case 6:
        setLEDs(LEDState, LOW, LEDState);
      break;
      case 7:
        setLEDs(LOW, LEDState, LEDState);
      break;
    }
    LEDState = !LEDState;       
  }
}
