const int buzzerpin=3;
int OnOffsound=2500;
int Resetsound=400;
//de adaugat var pt buzzer : ton la reset si ton la draw

const int pinSW = 2;
const int pinX = A0; 
const int pinY = A1;

// declare all the segments pins and states
const int pinA = 12;
const int pinB = 10;
const int pinC = 9;
const int pinD = 8;
const int pinE = 7;
const int pinF = 6;
const int pinG = 5;
const int pinDP = 4;
const int segSize = 8;

byte ledAState = LOW;
byte ledBState = LOW;
byte ledCState = LOW;
byte ledDState = LOW;
byte ledEState = LOW;
byte ledFState = LOW;
byte ledGState = LOW;
byte ledDPState = LOW;

int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

// declare variables for blinking state
int currentBlinkPin = 4;
byte blinkLedState = LOW;


// declare variables for joystick potentiometers
int xValue = 0;
int yValue = 0;
int minThreshold = 400;
int maxThreshold = 600;

byte joyBackToMiddle = LOW;

// declare variables for joystick switch
byte reading = LOW;
byte lastSwState = HIGH;
byte switchState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 50;

unsigned int blinkInterval = 400;
unsigned long lastBlinkTime = 0;

unsigned long pressedTime;
unsigned long releasedTime;

const int longPressDuration = 1000;
const int shortPressDuration = 50;

void setup() {
  for (int i = 0; i < segSize; i++) {
    pinMode(segments[i], OUTPUT);
  }
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(buzzerpin, OUTPUT);
  Serial.begin(9600);
}

// function that displays the state of all the segments
void displayLeds() {
  if (currentBlinkPin != pinA)
    digitalWrite(pinA, ledAState);
  if (currentBlinkPin != pinB)
    digitalWrite(pinB, ledBState);
  if (currentBlinkPin != pinC)
    digitalWrite(pinC, ledCState);
  if (currentBlinkPin != pinD)
    digitalWrite(pinD, ledDState);
  if (currentBlinkPin != pinE)
    digitalWrite(pinE, ledEState);
  if (currentBlinkPin != pinF)
    digitalWrite(pinF, ledFState);
  if (currentBlinkPin != pinG)
    digitalWrite(pinG, ledGState);
  if (currentBlinkPin != pinDP)
    digitalWrite(pinDP, ledDPState);
  digitalWrite(currentBlinkPin, blinkLedState);
}

// function that resets the state of all the segments
// and moves the blinking to the decimal point
void resetLeds() {
  ledAState = LOW;
  ledBState = LOW;
  ledCState = LOW;
  ledDState = LOW;
  ledEState = LOW;
  ledFState = LOW;
  ledGState = LOW;
  ledDPState = LOW;

  currentBlinkPin = pinDP;
}

// function that moves the blinking segment according to the movement of the joystick
void moveCurrentPosition() {
  if (yValue < minThreshold && joyBackToMiddle == LOW) {
    if (currentBlinkPin == pinA) {
      currentBlinkPin = pinF;
      Serial.println("left from A to F");
    }
    else if (currentBlinkPin == pinB) {
      currentBlinkPin = pinF;
      Serial.println("left from B to F");
    }
    else if (currentBlinkPin == pinC) {
      currentBlinkPin = pinE;
      Serial.println("left from C to E");
    }
    else if (currentBlinkPin == pinDP) {
      currentBlinkPin = pinC;
      Serial.println("left from DP to C");
    }
    else if (currentBlinkPin == pinD) {
      currentBlinkPin = pinE;
      Serial.println("left from D to E");
    }
    joyBackToMiddle = HIGH;
  }
  else if (yValue > maxThreshold && joyBackToMiddle == LOW) {
    if (currentBlinkPin == pinA || currentBlinkPin == pinF) {
      currentBlinkPin = pinB;
      Serial.println("right from A to B");
    }
    else if (currentBlinkPin == pinF) {
      currentBlinkPin = pinB;
      Serial.println("right from F to B");
    }
    else if (currentBlinkPin == pinC) {
      currentBlinkPin = pinDP;
      Serial.println("right from C to DP");
    }
    else if (currentBlinkPin == pinE) {
      currentBlinkPin = pinC;
      Serial.println("right from E to C");
    }
    else if (currentBlinkPin == pinD) {
      currentBlinkPin = pinC;
      Serial.println("right from D to C");
    }
    joyBackToMiddle = HIGH; 
  }
  else if (joyBackToMiddle == HIGH && yValue < maxThreshold && yValue > minThreshold) {
    joyBackToMiddle = LOW;
  }

  if (xValue < minThreshold && joyBackToMiddle == LOW) {
    if (currentBlinkPin == pinG) {
      currentBlinkPin = pinD;
      Serial.println("down from G to D");
    }
    else if (currentBlinkPin == pinA) {
      currentBlinkPin = pinG;
      Serial.println("down from A to G");
    }
    else if (currentBlinkPin == pinB) {
      currentBlinkPin = pinG;
      Serial.println("down from B to G");
    }
    else if (currentBlinkPin == pinF) {
      currentBlinkPin = pinG;
      Serial.println("down from F to G");
    }
    else if (currentBlinkPin == pinC) {
      currentBlinkPin = pinD;
      Serial.println("down from C to D");
    }
    else if (currentBlinkPin == pinE) {
      currentBlinkPin = pinD;
      Serial.println("down from E to D");
    }
    joyBackToMiddle = HIGH;
  }
  else if (xValue > maxThreshold && joyBackToMiddle == LOW) {
    if (currentBlinkPin == pinG) {
      currentBlinkPin = pinA;
      Serial.println("up from G to A");
    }
    else if (currentBlinkPin == pinD) {
      currentBlinkPin = pinG;
      Serial.println("up from D to G");
    }
    else if (currentBlinkPin == pinE) {
      currentBlinkPin = pinG;
      Serial.println("up from E to G");
    }
    else if (currentBlinkPin == pinC) {
      currentBlinkPin = pinG;
      Serial.println("up from C to G");
    }
    else if (currentBlinkPin == pinB) {
      currentBlinkPin = pinA;
      Serial.println("up from B to A");
    }
    else if (currentBlinkPin == pinF) {
      currentBlinkPin = pinA;
      Serial.println("up from F to A");
    }
    joyBackToMiddle = HIGH;
  }
  else if (joyBackToMiddle == HIGH && xValue < maxThreshold && xValue > minThreshold) {
    joyBackToMiddle == LOW;
  }
}

// function that changes led status of the current position or the if the entire display is reset based on how long the button is pressed ->(long press - reset, short press - change led state)
void buttonPressed() {
  if (reading != lastSwState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != switchState) {
      switchState = reading;
      if (switchState == LOW) {
        pressedTime = millis();
      }
      else {
        releasedTime = millis();
      }
      long pressDuration = releasedTime - pressedTime;
      if (pressDuration > longPressDuration) {
        Serial.println("long press");
        tone(buzzerpin, Resetsound, 600);
        resetLeds();
      }
      else if (pressDuration > shortPressDuration) {
        Serial.println("short press");
        tone(buzzerpin, OnOffsound, 300);
        if (currentBlinkPin == pinA) {
          ledAState = !ledAState;
        }
        else if (currentBlinkPin == pinB) {
          ledBState = !ledBState;
        }
        else if (currentBlinkPin == pinC) {
          ledCState = !ledCState;
        }
        else if (currentBlinkPin == pinD) {
          ledDState = !ledDState; 
        }
        else if (currentBlinkPin == pinE) {
          ledEState = !ledEState;
        }
        else if (currentBlinkPin == pinF) {
          ledFState = !ledFState;
        }
        else if (currentBlinkPin == pinG) {
          ledGState = !ledGState;
        }
        else if (currentBlinkPin == pinDP) {
          ledDPState = !ledDPState;
        }
            
      }
    }
  }
  
  lastSwState = reading;
}

void blink() {
  if (millis() - lastBlinkTime > blinkInterval) {
    blinkLedState = !blinkLedState;
    lastBlinkTime = millis();
  }
}

void loop() {
  displayLeds();

  xValue = analogRead(pinX);
  yValue = analogRead(pinY);

  blink();

  moveCurrentPosition();

  reading = digitalRead(pinSW);

  buttonPressed(); 
}