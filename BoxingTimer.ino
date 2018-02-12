#include "SevSeg.h"
SevSeg sevseg;

int LEDr = A3, // 1k in Serie gegen Ground
    LEDy = A4, // 1k in Serie gegen Ground
    LEDg = A5, // 1k in Serie gegen Ground
    box = A0, // gegen Ground
    button = A1, // 330 in Serie gegen Ground
    poti = A2, // Stützkondensator parallel

// box Variablen
    boxState = 0;
unsigned long tmpBox1 = 0,
              tmpBox2 = 0;

// Segment Variablen
int rundenAnzahl = 2,
    tRunde = 300,
    tPause = 100,

// CountToZero Variablen
    timeLeft = 0;

const unsigned long sekunde = 1000;
unsigned long tmpSekunde1 = 0,
              tmpSekunde2 = 0;

// blink LED Variablen
int blinkLED = 0,
    LEDState = 0,
    blinkCounter = 0;
unsigned long tmpBlink1 = 0,
              tmpBlink2 = 0;

// Button Variablen
int buttonState = 0,
    lastButtonState = 0,
    buttonStateMenu = 0,
    lastButtonStateMenu = 0;
unsigned long timeStamp = 0,
              timeStampMenu = 0;

// GetPotiValue Variablen
int potiValue = 0,
    tmp = 0;
const int tMax = 300,
          rMax = 16;

// Funktionen
int getPotiValue(int selection);
void beep(unsigned long a, unsigned long b, int anzahl, int displayNumber, int dec);
void countToZero(int Time);
void setLEDs(int setLEDg, int setLEDy, int setLEDr);
void segment(int tRunde, int tPause, int rundenAnzahl);

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
  buttonStateMenu = digitalRead(button);
  if (buttonStateMenu != lastButtonStateMenu && buttonStateMenu == HIGH) {
    switch(potiValue){
      case 1 :
        beep(20, 0, 1, tmp, 2);
        timeStampMenu = millis();
      break;
      case 2 :
        beep(20, 0, 1, tmp, 0);
        timeStampMenu = millis();
      break;
      case 3 :
        beep(20, 0, 1, tmp, 2);
        timeStampMenu = millis();
      break;
      case 4 :
        beep(20, 0, 1, tmp, 2);
        timeStampMenu = millis();    
      break;
    }
  }
  
  // Wenn button 600ms gedrückt, dann
  // Lese Werte ein Grün = Rundendauer
  // Rot = Pausendauer, Gelb = Anzahl an Runden
  // Alle = Start
  if (buttonStateMenu == lastButtonStateMenu && buttonStateMenu == HIGH) {
    if (millis() - timeStampMenu > 600) {
      switch (potiValue){
        case 1 :
          Beep(30, 0, 1, tmp, 2);
          tRunde = getPotiValue(0);
          timeStampMenu = millis();
        break;
        case 2 :
          Beep(30, 0, 1, tmp, 0);
          rundenAnzahl = getPotiValue(2);
          timeStampMenu = millis();
        break;
        case 3 :
          Beep(30, 0, 1, tmp, 2);
          tPause = getPotiValue(1);
          timeStampMenu = millis();
        break;
        case 4 :
          beep(30, 0, 1, tmp, 2);
          segment(tRunde, tPause, rundenAnzahl);
          timeStampMenu = millis();
        break;
      }
    }
  }
  lastButtonStateMenu = buttonStateMenu;
  
  // display PotiValue on LEDs and
  // tRunde/tPause/rundenAnzahl/Gesamtzeit on 7-Seg
  potiValue = map(analogRead(poti), 0, 1023, 1, 4);
  switch (potiValue){
    case 1 :
      SetLEDs(HIGH, LOW, LOW);
      sevseg.setNumber(tRunde, 2);
    break;
    case 2 :
      SetLEDs(LOW, HIGH, LOW);
      sevseg.setNumber(rundenAnzahl, 0);
    break;
    case 3 :
      SetLEDs(LOW, LOW, HIGH);
      sevseg.setNumber(tPause, 2);
    break;
    case 4 :
      tmpBlink1 = millis();
    
      if((tmpBlink1 - tmpBlink2) >= 250){
        
        tmpBlink2 = millis();
        
        if(blinkCounter % 2 == 0 && LEDState == LOW){
          setLEDs(HIGH, HIGH, HIGH);
          LEDState = HIGH;
        }
        else if(blinkCounter % 2 == 1 && LEDState == HIGH){
          setLEDs(LOW, LOW, LOW);
          LEDState = LOW;
        }
        blinkCounter += 1;              
      }
      tmp = 100*((tRunde/100)*rundenAnzahl) + 100*(((tRunde%100)*rundenAnzahl)/60) + ((tRunde%100)*rundenAnzahl)%60;
      if(rundenAnzahl > 1){
        tmp += 100*((tPause/100)*(rundenAnzahl-1)) + 100*(((tPause%100)*(rundenAnzahl-1))/60) + ((tPause%100)*(rundenAnzahl-1))%60;
      }
      sevseg.setNumber(tmp, 2);    
    break;
  }
  
  sevseg.refreshDisplay();
}

void segment(int tRunde, int tPause, int rundenAnzahl){
  static int k = 0;
  static int t1 = 0;
  static int t2 = 0;
  static int T = 0;
  
  t1 = tRunde;
  t2 = tPause;
  T = rundenAnzahl;
  
  for (k = 1; k <= T; k++) {
    
    // Beep für start der Runde
    beep(500, 200, 5, t1, 2);
    
    // Runde verstreichen lassen
    setLEDs(HIGH, LOW, LOW);
    countToZero(t1);
    
    // Beep Start der Pause
    beep(100, 250, 5, 0, 2);
    
    // Pause verstreichen lassen
    // wenn nicht letzte runde
    if (k < T){
      setLEDs(LOW, LOW, HIGH);
      countToZero(t2);
    }
  }
}

void setLEDs(int setLEDg, int setLEDy, int setLEDr){
  digitalWrite(LEDr, setLEDr);
  digitalWrite(LEDg, setLEDg);
  digitalWrite(LEDy, setLEDy);
}

void beep(unsigned long a, unsigned long b, int anzahl, int displayNumber, int dec) {
  // a=länge v beep, b=pause zw beep, anzahl=Anzahl der Beeps
  static int i = 0;
  static int c = 0;
  c = dec;
  i = anzahl;
  tmpBox1 = 0;
  tmpBox2 = millis();

  digitalWrite(box, HIGH);
  boxState = HIGH;

  while(true){
    sevseg.setNumber(displayNumber, c);
    sevseg.refreshDisplay();
    tmpBox1 = millis();
    if((tmpBox1 - tmpBox2) >= b && boxState == LOW){
      tmpBox2 = millis();
      digitalWrite(box, HIGH);
      boxState = HIGH;
    }
    else if((tmpBox1 - tmpBox2) >=a && boxState == HIGH){
      tmpBox2 = millis();
      digitalWrite(box, LOW);
      boxState = LOW;
      i -= 1;            
    }  
    if(i == 0){
      digitalWrite(box, LOW);
      break;
    }          
  }
}

void countToZero(int Time) {
  tmpBlink1 = 0;
  tmpBlink2 = 0;
  blinkCounter = 0;
  
  timeLeft = Time;
  tmpSekunde1=0;
  tmpSekunde2=0;

  // Bleibe in der Schleife bis Time verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (true) {
    tmpSekunde1 = millis();

    // Zähle Sekunden
    if(tmpSekunde1 - tmpSekunde2 >= sekunde){
      
      timeLeft -= 1;
      tmpSekunde2 = millis();

      // Gelbe LED leuchtet letzten 30 sekunden
      if(timeLeft == 30){
        SetLEDs(LOW, HIGH, LOW);
      }
      
      // LED blinkt letzten 10 sekunden
      if(timeLeft <=10){
        tmpBlink1 = millis();
      
        if((tmpBlink1 - tmpBlink2) >= 250){
          
          tmpBlink2 = millis();
          
          if(blinkCounter % 2 == 0 && LEDState == LOW){
            setLEDs(LOW, HIGH, LOW);
            LEDState = HIGH;
          }
          else if(blinkCounter % 2 == 1 && LEDState == HIGH){
            setLEDs(LOW, LOW, LOW);
            LEDState = LOW;
          }
          blinkCounter += 1;              
        }
      }

      // Ton Bei 10, 5, 4, 3, 2, 1 sekunden
      if(timeLeft <= 5 || timeLeft == 10){
        beep(100, 0, 1, timeLeft, 2);
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
      if(blinkCounter % 2 == 0 && LEDState == LOW){
        digitalWrite(blinkLED, HIGH);
        LEDState = HIGH;
        tmpBlink2 = millis();
      }
      else if(blinkCounter % 2 == 1 && LEDState == HIGH){
        SetLEDs(LOW, LOW, LOW);
        LEDState = LOW;
        tmpBlink2 = millis();
      }
        blinkCounter += 1;              
    }
    
    // Button press
    buttonState = digitalRead(button);
    if (buttonState != lastButtonState && buttonState == HIGH) {
      timeStamp = millis();
      beep(20, 0, 1, tmp, 2);
    }

    // Wenn button 600ms gedrückt, dann return potiValue
    if (buttonState == lastButtonState && buttonState == HIGH) {
      if (millis() - timeStamp > 600) {
        beep(30, 0, 1, tmp, 2);
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
