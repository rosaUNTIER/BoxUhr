#include "SevSeg.h"
SevSeg sevseg;

int LEDr = A3; // 1k in Serie gegen Ground
int LEDy = A4; // 1k in Serie gegen Ground
int LEDg = A5; // 1k in Serie gegen Ground
int box = A0; // gegen Ground
int button = A1; // 330 in Serie gegen Ground
int poti = A2; // Stützkondensator parallel

int exitStatus = 0;

// Segment Variablen
int rundenAnzahl = 2;
int tRunde = 300;
int tPause = 100;

const unsigned long sekunde = 1000;

int blinkLED = 0;

unsigned long timeStampMenu = 0;  // timestamp für menü

// Poti Variablen
int lastPotiValue = 9999;
int potiValue = 0;
int tmp = 0;
const int tMax = 300; // maximum in zeit 300 = 3:00
const int rMax = 16;  // maximum für rundenAnzahl

// StoppUhr Variablen
int timeCount = 0; 
int timeFreeze = 0;
int *timeToDisplay = NULL;

// Funktionen
void main_Sets(boolean game);
int setSetsParameter(int selection);
void runSets(int tRunde, int tPause, int rundenAnzahl);

void main_Stoppuhr();
void runStoppuhr();

void main_Game();
void runGame(int tRunde, int tPause, int rundenAnzahl, int startLevel);

void countToZero(int Time, int selection, int level);
void hold();

int buttonClick();

void setLEDs(int setLEDg, int setLEDy, int setLEDr);
void blinkLEDs(int selection);

int countInterval(unsigned long interval, int reset);
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

  //Serial.begin(9600);
}

// Home Screen
void loop() {
  potiValue = map(analogRead(poti), 0, 1023, 1, 3);
  switch(potiValue){
    case 1:
      sevseg.setChars("sets");
      setLEDs(HIGH, LOW, LOW);
      if (buttonClick() == 3) main_Sets(false);
    break;
    case 2:
      sevseg.setChars("stop");
      setLEDs(LOW, HIGH, LOW);
      if (buttonClick() == 3) main_Stoppuhr();
    break;
    case 3: 
      sevseg.setChars("rctn");
      setLEDs(LOW, LOW, HIGH);
      if (buttonClick() == 3) main_Game();
    break;
  }
  sevseg.refreshDisplay();
}

// Main funktion für Sets
void main_Sets(boolean game) {
  static int maxPotiValue = 4;
  exitStatus = 0;
  if (game == true) maxPotiValue = 3;
  else maxPotiValue = 4;
  timeStampMenu = millis();
  
  while (true){
    
    potiValue = map(analogRead(poti), 0, 1023, 0, maxPotiValue);
    if (potiValue != lastPotiValue){
      timeStampMenu = millis();
      if (potiValue == 4 || (potiValue == 0 && game == true)){
        tmp = 100*((tRunde/100)*rundenAnzahl) + 100*(((tRunde%100)*rundenAnzahl)/60) + ((tRunde%100)*rundenAnzahl)%60;
          if(rundenAnzahl > 1){
            tmp += 100*((tPause/100)*(rundenAnzahl-1)) + 100*(((tPause%100)*(rundenAnzahl-1))/60) + ((tPause%100)*(rundenAnzahl-1))%60;
          }     
      }
    }
    lastPotiValue = potiValue;
    
    switch (potiValue){
      case 0: 
        setLEDs(LOW, LOW, LOW);
        if (millis() - timeStampMenu < 500 && game == true) sevseg.setNumber(tmp, 2);
        else if (millis() - timeStampMenu < 500 && game == false) sevseg.setChars("sets");
        else sevseg.setChars("back");
        if (buttonClick() == 3) exitStatus = 1;
      break;
      case 1: 
        setLEDs(HIGH, LOW, LOW);
        if (millis() - timeStampMenu < 500) sevseg.setChars("rnde");
        else sevseg.setNumber(tRunde, 2);
        if (buttonClick() == 3) tRunde = setSetsParameter(0);
      break;
      case 2:
        setLEDs(LOW, HIGH, LOW);      
        if (millis() - timeStampMenu < 500) sevseg.setChars("rndn");
        else sevseg.setNumber(rundenAnzahl);
        if (buttonClick() == 3) rundenAnzahl = setSetsParameter(2);
      break;
      case 3:
        setLEDs(LOW, LOW, HIGH);
        if (millis() - timeStampMenu < 500) sevseg.setChars("paus");
        else sevseg.setNumber(tPause, 2);
        if (buttonClick() == 3) tPause = setSetsParameter(1);
      break;
      case 4:
        blinkLEDs(4);
        if (millis() - timeStampMenu < 500) sevseg.setChars(" go ");
        else sevseg.setNumber(tmp, 2); 
        if (buttonClick() == 3) runSets(tRunde, tPause, rundenAnzahl);
      break;
    }
    
    if (exitStatus == 1) break;
    sevseg.refreshDisplay();
  }
}

// Lese Werte von Poti ein
int setSetsParameter(int selection) {
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
    
    // Wenn button 600ms gedrückt, dann return potiValue
    if (buttonClick() == 3) return tmp;
    
    // display PotiValue
    potiValue = map(analogRead(poti), 0, 1023, 1, bound);
    tmp = (potiValue / 60) * 100 + (potiValue % 60);   
    sevseg.setNumber(tmp, 2);
    sevseg.refreshDisplay();
  }
}

// Starte Sets
void runSets(int tRunde, int tPause, int rundenAnzahl) {
  static int k = 0;
  static int T = 0;

  exitStatus = 0;
  T = rundenAnzahl;
  
  for (k = 1; k <= T; k++) {
    
    // Runde verstreichen lassen
    setLEDs(HIGH, LOW, LOW);
    countToZero(tRunde, 0, 0);
    if (exitStatus == 1) break;

    // Pause verstreichen lassen
    // wenn nicht letzte runde
    if (k < T){
      setLEDs(LOW, LOW, HIGH);
      countToZero(tPause, 1, 0);
      if (exitStatus == 1) break;
    }
  }
  beep(100, 250, 5);
}

// Main funktion für Stoppuhr
void main_Stoppuhr() {
  exitStatus = 0;
  
  while (true){
    potiValue = map(analogRead(poti), 0, 1023, 0, 1);
    if (potiValue != lastPotiValue) timeStampMenu = millis();
    lastPotiValue = potiValue;
    switch (potiValue){
      case 0:
        if (millis() - timeStampMenu < 500) sevseg.setChars("stop");
        else sevseg.setChars("back");
        setLEDs(LOW, LOW, LOW);
        if (buttonClick() == 3) exitStatus = 1;
      break;
      case 1:
        sevseg.setChars(" go ");
        blinkLEDs(4);
        if (buttonClick() == 3) runStoppuhr();
        exitStatus = 0;
      break;
    }
    if (exitStatus == 1) break;
    sevseg.refreshDisplay();
  }
}

// Starte Stoppuhr
void runStoppuhr() {
  setLEDs(HIGH, LOW, LOW);
  timeCount = 0;
  timeFreeze = 0;
  timeToDisplay = &timeCount;
  exitStatus = 0;
  countInterval(1000, 1);
  
  while (true) {
    
    // Zähle Sekunden
    timeCount += countInterval(1000, 0);
    // Überspringe _60 - _99
    if ((timeCount % 100) >= 60){
        timeCount += 40;
    }


    switch (buttonClick()){
    // Button press
      // Weiter
      case 1:
        if (timeToDisplay == &timeFreeze){
          timeToDisplay = &timeCount;
          setLEDs(HIGH, LOW, LOW);
        }
        
        // Lap
        else if (timeToDisplay == &timeCount){
          timeFreeze = timeCount;
          timeToDisplay = &timeFreeze;
          setLEDs(LOW, HIGH, LOW);
        }
        break;
      case 3:
        if (timeToDisplay == &timeFreeze){
          setLEDs(LOW, LOW, HIGH);
  
          // eigentliche Pause
          while (true){
            sevseg.setNumber(*timeToDisplay, 2);
            sevseg.refreshDisplay();
            
            // Continue: bei buttonClick break und reset countInterval
            if (buttonClick() == 1){
              setLEDs(HIGH, LOW, LOW);
              countInterval(1000, 1);
              timeToDisplay = &timeCount;
              break;
            }
          }
        }
        else if (timeToDisplay == &timeCount){
          exitStatus = 1;
        }
        break;      
    }
    
    // Display timeToDisplay
    sevseg.setNumber(*timeToDisplay, 2);
    sevseg.refreshDisplay();
    if (exitStatus == 1) break;
  }
}

// Main funktion für das Game
void main_Game() {
  static int level = 1;
  exitStatus = 0;
  timeStampMenu = millis();
  
  while (true){
   
    potiValue = map(analogRead(poti), 0, 1023, 0, 3);
    if (potiValue != lastPotiValue){
      timeStampMenu = millis();
      // Berechne Gesamtzeit
      if (potiValue == 3 || potiValue == 1){
        tmp = 100*((tRunde/100)*rundenAnzahl) + 100*(((tRunde%100)*rundenAnzahl)/60) + ((tRunde%100)*rundenAnzahl)%60;
          if(rundenAnzahl > 1){
            tmp += 100*((tPause/100)*(rundenAnzahl-1)) + 100*(((tPause%100)*(rundenAnzahl-1))/60) + ((tPause%100)*(rundenAnzahl-1))%60;
          }     
      }
    }
    lastPotiValue = potiValue;
    
    switch (potiValue){
      case 0: 
        setLEDs(LOW, LOW, LOW);
        if (millis() - timeStampMenu < 500) sevseg.setChars("rctn");
        else sevseg.setChars("back");
        if (buttonClick() == 3) exitStatus = 1;
      break;
      case 1: 
        setLEDs(HIGH, LOW, LOW);
        if (millis() - timeStampMenu < 500) sevseg.setChars("sets");
        else sevseg.setNumber(tmp, 2);
        if (buttonClick() == 3){
          main_Sets(true); 
          timeStampMenu = millis();
        }
        exitStatus = 0;
      break;
      case 2:
        setLEDs(LOW, HIGH, LOW);      
        if (millis() - timeStampMenu < 500) sevseg.setChars("lvl");
        else sevseg.setNumber(level);
        if (buttonClick() == 3){
          level = setSetsParameter(2); 
          timeStampMenu = millis();
        }
      break;
      case 3:
        blinkLEDs(4);
        if (millis() - timeStampMenu < 500) sevseg.setChars(" go ");
        else sevseg.setNumber(tmp, 2); 
        if (buttonClick() == 3) runGame(tRunde, tPause, rundenAnzahl, level);
      break;
    }
    
    if (exitStatus == 1) break;
    sevseg.refreshDisplay();
  }
}

// Starte das Game
void runGame(int tRunde, int tPause, int rundenAnzahl, int Level){
  static int k = 0;
  static int T = 0;

  exitStatus = 0;
  T = rundenAnzahl;
  
  for (k = 1; k <= T; k++) {
    
    // Runde verstreichen lassen
    setLEDs(HIGH, LOW, LOW);
    countToZero(tRunde, 0, Level);
    if (exitStatus == 1) break;

    // Pause verstreichen lassen
    // wenn nicht letzte runde
    if (k < T){
      setLEDs(LOW, LOW, HIGH);
      countToZero(tPause, 1, 0);
      if (exitStatus == 1) break;
    }
  }
  beep(100, 250, 5);
}

// Zähle von Time bis null(in sekunden)
// wenn level > 0 dann läuft das game
void countToZero(int Time, int selection, int level) {  
  static int timeLeft = 0;

  exitStatus = 0;
  timeLeft = Time;
  sevseg.setNumber(timeLeft, 2);
  countInterval(1000, 1);
  
  if (selection == 0)  beep(500, 200, 5);
  else  beep(100, 250, 5);
  
  // Bleibe in der Schleife bis Time verstrichen ist 
  // und Zeige verbleidende Zeit auf Display
  while (true) {

    // Wenn button 600ms gedrückt, dann hold()
    if (buttonClick() == 3) {
        hold();
        if (exitStatus == 1) break;
        countInterval(1000, 1);
        sevseg.setNumber(timeLeft, 2);
        if (timeLeft <= 30) setLEDs(LOW, HIGH, LOW);
        else if (selection == 0) setLEDs(HIGH, LOW, LOW);
        else if (selection == 1) setLEDs(LOW, LOW, HIGH);
    }

    if (level > 0) randomBeep(2500 - level * 200, 5500 - level * 300, level);
    
    // Zähle Sekunden
    tmp = countInterval(1000, 0);

    if(tmp > 0){
      
      timeLeft -= tmp;

      // Ton Bei 10, 3, 2, 1 sekunden
      if(timeLeft == 10 && selection == 0 && level == 0){
        beep(233, 100, 3);
      }
      if(timeLeft <= 3 && selection == 1){
        beep(100, 0, 1);
      }

      // Überspringe _60 - _99
      if((timeLeft % 100) > 60){
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

// Warte bis longClick, bei doppelclick goto home
void hold() {
  
  while(true){

    // "halt" on 7Seg
    sevseg.setChars("halt");
    sevseg.refreshDisplay();

    // Blink
    blinkLEDs(4);
    
    if (buttonClick() == 2){
      exitStatus = 1;
      break;
    }
    
    // Wenn button 600ms gedrückt, dann break
    if (buttonClick() == 3) break;
  }
}

// zählt wieviele intervalle vergangen sind, bei reset wird der counter auf 0 gesetzt
int countInterval(unsigned long interval, int reset) {
  static unsigned long tmpInterval1 = millis();
  static unsigned long tmpInterval2 = millis();
  static long Zeit = 0;
  static unsigned long Interval = 0;

  if(reset == 1){
    tmpInterval2 = millis();
    Interval = interval;
  }
  
  // interval in milliseconds
  tmpInterval1 = millis();
  if (tmpInterval1 - tmpInterval2 > Interval){
    Zeit = long((tmpInterval1 - tmpInterval2) / Interval);
    tmpInterval2 = millis();
  }
  else Zeit = 0;

  return Zeit;
}

// Wenn Button gedrückt dann
// return 1 für Click
// return 2 für Double-Clik (innerhalb 0,4s)
// return 3 für Long-Click (0,6s gedrückt halten)
int buttonClick() {

static unsigned long tButton = 0;
static unsigned long tButton_release = 0;
static int returnValue = 0;
static int buttonState = LOW;
static int lastButtonState = LOW;

  returnValue = 0;
  buttonState = digitalRead(button);
  
  if (buttonState != lastButtonState && buttonState == HIGH && (millis() - tButton) > 400) {
   beep(20, 0, 1);
   tButton = millis();
   returnValue = 1;
  }
  else if (buttonState != lastButtonState && buttonState == HIGH && (millis() - tButton) <= 300) {
   beep(30, 0, 1);
   tButton = millis();
   returnValue = 2;
  }
  if (buttonState == lastButtonState && buttonState == HIGH && (millis() - tButton) > 600) {
   beep(40, 0, 1);
   tButton = millis();
   returnValue = 3;
  }
  
  lastButtonState = buttonState;
  
  return returnValue;
}

// Setzt alle LEDs
void setLEDs(int setLEDg, int setLEDy, int setLEDr) {
  digitalWrite(LEDr, setLEDr);
  digitalWrite(LEDg, setLEDg);
  digitalWrite(LEDy, setLEDy);
}

// Blinkt je nach input eine/mehrere LEDs 2 mal pro sekunde
void blinkLEDs(int selection) {
  static unsigned long tmpBlink1 = 0;
  static unsigned long tmpBlink2 = 0;
  static boolean LEDState = false;
  
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

// a=länge v beep, b=pause zw beep, anzahl=Anzahl der Beeps
void beep(unsigned long a, unsigned long b, int anzahl) {
  
  static int i = 0;
  i = anzahl;
  static unsigned long tmpBox = 0;
  static boolean boxState = false;

  boxState = true;
  digitalWrite(box, boxState);  
  sevseg.setChars("beep");
  tmpBox = millis();
  while(true){
    sevseg.refreshDisplay();

    if(i == 0){
      break;
    } 
    else if((millis() - tmpBox) > b && boxState == false){
      sevseg.setChars("beep");
      tmpBox = millis();
      boxState = !boxState;
      digitalWrite(box, boxState);
    }
    else if((millis() - tmpBox) > a && boxState == true){
      sevseg.setChars("");
      tmpBox = millis();
      boxState = !boxState;
      digitalWrite(box, boxState);
      i -= 1;            
    }          
  }
}

// nach random zeit zwischen lBound und uBound --> Beep
void randomBeep(unsigned long lBound, unsigned long uBound, int level){
  
  static unsigned long t = 0;
  static unsigned long tRun = 0;
  static int combo = 0;
  
  if (t == 0) {
    t = millis();
    tRun = random(lBound, uBound);
  }
  else if (millis() - t > tRun){
    combo = (millis() % level) + 1;
    beep(70, 110, combo);
    t = 0;
  }
}
