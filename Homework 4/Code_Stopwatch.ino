// Define connections to the shift register
const int latchPin = 11; // Connects to STCP (latch pin) on the shift register
const int clockPin = 10; // Connects to SHCP (clock pin) on the shift register
const int dataPin = 12; // Connects to DS (data pin) on the shift register

// Define connections to the digit control pins for a 4-digit display
const int segD1 = 4;
const int segD2 = 5;
const int segD3 = 6;
const int segD4 = 7;

// Define button pins with debounce intervals
const int buttonStartPause = 2;
const int buttonReset = 3;
const int buttonSaveLap = 8; // Change this to the pin you use for the lap button
const int debounceInterval = 50; // milliseconds

// Store the digits in an array for easy access
int displayDigits[] = {segD1, segD2, segD3, segD4};
const int displayCount = 4; // Number of digits in the display

// Define the number of unique encodings (0-9, A-F for hexadecimal)
const int encodingsNumber = 16;

// Define byte encodings for the hexadecimal characters 0-F
byte byteEncodings[encodingsNumber] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B11101110, // A
  B00111110, // b
  B10011100, // C
  B01111010, // d
  B10011110, // E
  B10001110 // F
};

// Variables for controlling the display update timing
unsigned long lastIncrement = 0;
unsigned long delayCount = 100; // Delay between updates (milliseconds)
unsigned long number = 0; // The number being displayed
unsigned long lapTimes[4] = {0}; // Array to store lap times
int currentLap = 0; // Index of the current lap time
bool isCounting = false; // Flag to indicate whether the stopwatch is counting
bool isPaused = false; // Flag to indicate whether the stopwatch is paused
bool isLapViewMode = false; // Flag to indicate whether the stopwatch is in lap view mode

// Variables for button debouncing
unsigned long lastButtonPress[3] = {0}; // Last time each button was pressed

void setup() {
  // Initialize the pins connected to the shift register as outputs
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // Initialize digit control pins and set them to LOW (off)
  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }

  // Initialize button pins
  pinMode(buttonStartPause, INPUT_PULLUP);
  pinMode(buttonReset, INPUT_PULLUP);
  pinMode(buttonSaveLap, INPUT_PULLUP);

  // Attach interrupts to the lap button and pause button
  attachInterrupt(digitalPinToInterrupt(buttonSaveLap), saveLap, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonStartPause), startPause, FALLING);

  // Begin serial communication for debugging purposes
  Serial.begin(9600);
}

void loop() {
  // Check for button presses and debounce
  checkButton(buttonStartPause, startPause);
  checkButton(buttonReset, resetStopwatch);
  checkButton(buttonSaveLap, saveLap);

  if (isCounting && !isPaused && !isLapViewMode) {
    // Increment the number if the stopwatch is counting and not paused
    if (millis() - lastIncrement > delayCount) {
      number++;
      lastIncrement = millis();
    }
    // Check for minute rollover
    if (number >= 6000) { // 6000 tenths of a second is equivalent to 10 minutes
      number = 0; // Reset to 0 after 10 minutes
    }
  }

  // Display the current time or lap time on the 7-segment display
  if (isCounting && !isLapViewMode) {
    writeNumber(number);
  } else if (isLapViewMode) {
    // Display the current lap time when in lap view mode
    writeNumber(lapTimes[currentLap]);
  }
}

void writeReg(int digit) {
  // Prepare to shift data by setting the latch pin low
  digitalWrite(latchPin, LOW);

  // Shift out the byte representing the current digit to the shift register
  shiftOut(dataPin, clockPin, MSBFIRST, digit);

  // Latch the data onto the output pins by setting the latch pin high
  digitalWrite(latchPin, HIGH);
}

void activateDisplay(int displayNumber) {
  // Turn off all digit control pins to avoid ghosting
  for (int i = 0; i < displayCount; i++) {
    digitalWrite(displayDigits[i], HIGH);
  }

  // Turn on the current digit control pin
  digitalWrite(displayDigits[displayNumber], LOW);
}

void writeNumber(unsigned long number) {
  // Initialize necessary variables for tracking the current number and digit position
  unsigned long currentNumber = number;
  int displayDigit = 3; // Start with the least significant digit
  int lastDigit = 0;

  // Loop through each digit of the current number
  while (currentNumber != 0 || displayDigit >= 0) {
    // Extract the last digit of the current number
    lastDigit = currentNumber % 10;

    // Activate the current digit on the display
    activateDisplay(displayDigit);

    // Output the byte encoding for the last digit to the display with decimal point
    writeReg(byteEncodings[lastDigit] | (displayDigit == 2 ? B00000001 : B00000000));

    // Implement a delay if needed for multiplexing visibility
    delay(1); // A small delay to visualize multiplexing

    // Clear the display to prevent ghosting between digit activations
    writeReg(B00000000); // Clear the register to avoid ghosting

    // Move to the next digit
    displayDigit--;

    // Update 'currentNumber' by removing the last digit
    currentNumber /= 10;
  }
}
void startPause() {
  // Start or pause the stopwatch
  if (millis() - lastButtonPress[0] > debounceInterval) {
    if (!isCounting) {
      // Start counting
      isCounting = true;
      isPaused = false;
      isLapViewMode = false;
    } else {
      // Pause counting
      isPaused = !isPaused;
      if (isPaused) {
        isLapViewMode = false; // Exiting lap view mode on pause
      }
    }
    lastButtonPress[0] = millis();
  }
}

void saveLap() {
  // Save lap time or enter lap view mode
  if (millis() - lastButtonPress[2] > debounceInterval) {
    if (isCounting && !isPaused) {
      // Save lap time if counting and not paused
      lapTimes[currentLap] = number;
      currentLap = (currentLap + 1) % 4; // Cycle through lap times (up to 4 laps)
    } else if (isPaused) {
      // Enter or exit lap view mode when paused
      isLapViewMode = !isLapViewMode;
      if (isLapViewMode) {
        currentLap = 0; // Reset to the first lap when entering lap view mode
      }
    }
    lastButtonPress[2] = millis();
  }
}

void resetStopwatch() {
  // Reset the stopwatch and lap times
  if (isPaused && millis() - lastButtonPress[1] > debounceInterval) {
    isCounting = false;
    isPaused = false;
    isLapViewMode = false;
    number = 0;
    currentLap = 0;
    for (int i = 0; i < 4; i++) {
      lapTimes[i] = 0;
    }
    lastButtonPress[1] = millis();
  }
}

void checkButton(int buttonPin, void (*buttonFunction)()) {
  // Check if a button is pressed and call the corresponding function
  if (digitalRead(buttonPin) == LOW && millis() - lastButtonPress[buttonPin] > debounceInterval) {
    buttonFunction();
    lastButtonPress[buttonPin] = millis();
  }
}
