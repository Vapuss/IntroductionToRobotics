#include "pitches.h"

#define CHOICE_OFF      0 //Used to control LEDs
#define CHOICE_NONE     0 //Used to check buttons
#define CHOICE_RED  (1 << 0)
#define CHOICE_GREEN    (1 << 1)
#define CHOICE_BLUE (1 << 2)
#define CHOICE_YELLOW   (1 << 3)

const int buttonRed = 12;           
const int buttonYellow = 10;          
const int buttonGreen = 8;          
const int buttonBlue = 6;        
  
const int ledRed = 13;             
const int ledYellow = 11;             
const int ledGreen = 9;             
const int ledBlue = 7;    
       
const int buzzer = 5;   

const int rgbLedBlue = 4; 
const int rgbLedRed = 3; 
const int rgbLedGreen = 2; 



// Define game parameters
#define ROUNDS_TO_WIN      10 //Number of rounds to succesfully remember before you win. 13 is do-able.
#define ENTRY_TIME_LIMIT   3000 //Amount of time to press a button before game times out. 3000ms = 3 sec



// Game state variables
byte gameBoard[32]; //Contains the combination of buttons as we advance
byte gameRound = 0; //Counts the number of succesful rounds the player has made it through

void setup()
{
  //Setup hardware inputs/outputs. These pins are defined in the hardware_versions header file

  //Enable pull ups on inputs
  pinMode(buttonRed, INPUT_PULLUP);
  pinMode(buttonYellow, INPUT_PULLUP);
  pinMode(buttonGreen, INPUT_PULLUP);
  pinMode(buttonBlue, INPUT_PULLUP);

  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);


  
  pinMode(rgbLedBlue, OUTPUT);
  pinMode(rgbLedRed, OUTPUT);
  pinMode(rgbLedGreen, OUTPUT);
}

void loop()
{
  attractMode(); // Blink lights while waiting for user to press a button

  // Indicate the start of game play
  setLEDs(CHOICE_RED | CHOICE_GREEN | CHOICE_BLUE | CHOICE_YELLOW); // Turn all LEDs on
  delay(1000);
  setLEDs(CHOICE_OFF); // Turn off LEDs
  delay(250);

 
    // Play memory game and handle result
    if (play_memory() == true) 
      play_winner(); // Player won, play winner tones
    else 
      play_loser(); // Player lost, play loser tones
  

  
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//The following functions are related to game play only

// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
boolean play_memory(void)
{
  randomSeed(millis()); // Seed the random generator with random amount of millis()

  gameRound = 0; // Reset the game to the beginning

  while (gameRound < ROUNDS_TO_WIN) 
  {
    add_to_moves(); // Add a button to the current moves, then play them back

    playMoves(); // Play back the current game board

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {
      byte choice = wait_for_button(); // See what button the user presses

      if (choice == 0) return false; // If wait timed out, player loses

      if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses
    }

    delay(1000); // Player was correct, delay before playing moves
  }

  return true; // Player made it through all the rounds to win!
}

void playMoves(void)
{
  for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++) 
  {
    toner(gameBoard[currentMove], 150);

    // Wait some amount of time between button playback
    // Shorten this to make game harder
    delay(150); // 150 works well. 75 gets fast.
  }
}

void add_to_moves(void)
{
  byte newButton = random(0, 4); //min (included), max (exluded)

  // We have to convert this number, 0 to 3, to CHOICEs
  if(newButton == 0) newButton = CHOICE_RED;
  else if(newButton == 1) newButton = CHOICE_GREEN;
  else if(newButton == 2) newButton = CHOICE_BLUE;
  else if(newButton == 3) newButton = CHOICE_YELLOW;

  gameBoard[gameRound++] = newButton; // Add this new button to the game array
}

void setLEDs(byte leds)
{
  if ((leds & CHOICE_RED) != 0)
    digitalWrite(ledRed, HIGH);
  else
    digitalWrite(ledRed, LOW);

  if ((leds & CHOICE_GREEN) != 0)
    digitalWrite(ledGreen, HIGH);
  else
    digitalWrite(ledGreen, LOW);

  if ((leds & CHOICE_BLUE) != 0)
    digitalWrite(ledBlue, HIGH);
  else
    digitalWrite(ledBlue, LOW);

  if ((leds & CHOICE_YELLOW) != 0)
    digitalWrite(ledYellow, HIGH);
  else
    digitalWrite(ledYellow, LOW);
}

// Wait for a button to be pressed. 
// Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
byte wait_for_button(void)
{
  long startTime = millis(); // Remember the time we started the this loop

  while ( (millis() - startTime) < ENTRY_TIME_LIMIT) // Loop until too much time has passed
  {
    byte button = checkButton();

    if (button != CHOICE_NONE)
    { 
      toner(button, 150); // Play the button the user just pressed

      while(checkButton() != CHOICE_NONE) ;  // Now let's wait for user to release button

      delay(10); // This helps with debouncing and accidental double taps

      return button;
    }

  }

  return CHOICE_NONE; // If we get here, we've timed out!
}

// Returns a '1' bit in the position corresponding to CHOICE_RED, CHOICE_GREEN, etc.
const long debounceDelay = 50; // 50 milliseconds for debounce delay

// Variables to store the last time the buttons were pressed
unsigned long lastButtonRedTime = 0;
unsigned long lastButtonYellowTime = 0;
unsigned long lastButtonGreenTime = 0;
unsigned long lastButtonBlueTime = 0;

// Modified checkButton function with debouncing
byte checkButton(void)
{
  unsigned long currentMillis = millis();
  
  // Check each button
  if (digitalRead(buttonRed) == 0) {
    if (currentMillis - lastButtonRedTime > debounceDelay) {
      lastButtonRedTime = currentMillis;
      return CHOICE_RED;
    }
  } 
  else if (digitalRead(buttonGreen) == 0) {
    if (currentMillis - lastButtonGreenTime > debounceDelay) {
      lastButtonGreenTime = currentMillis;
      return CHOICE_GREEN;
    }
  } 
  else if (digitalRead(buttonBlue) == 0) {
    if (currentMillis - lastButtonBlueTime > debounceDelay) {
      lastButtonBlueTime = currentMillis;
      return CHOICE_BLUE;
    }
  } 
  else if (digitalRead(buttonYellow) == 0) {
    if (currentMillis - lastButtonYellowTime > debounceDelay) {
      lastButtonYellowTime = currentMillis;
      return CHOICE_YELLOW;
    }
  }

  return CHOICE_NONE; // If no button is pressed, return none
}

// Play tone with a specific frequency and duration for passive buzzer
void buzz_sound(int buzz_length_ms, int frequency)
{
  tone(buzzer, frequency, buzz_length_ms); 
  delay(buzz_length_ms); // Wait for the duration of the buzz
  noTone(buzzer); // Stop any tone playing
}

// Light an LED and play tone
void toner(byte which, int buzz_length_ms)
{
  setLEDs(which); //Turn on a given LED

  //Play the sound associated with the given LED
  switch(which) 
  {
    case CHOICE_RED:
      buzz_sound(buzz_length_ms, 440); // Originally 1136 us
      break;
    case CHOICE_GREEN:
      buzz_sound(buzz_length_ms, 880); // Originally 568 us
      break;
    case CHOICE_BLUE:
      buzz_sound(buzz_length_ms, 587); // Originally 851 us
      break;
    case CHOICE_YELLOW:
      buzz_sound(buzz_length_ms, 784); // Originally 638 us
      break;
  }

  setLEDs(CHOICE_OFF); // Turn off all LEDs
}


// Play the winner sound and lights
void play_winner(void)
{
  setRGBLed(0, 255, 0); // Set RGB LED to green
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  winner_sound();
  setLEDs(CHOICE_GREEN | CHOICE_BLUE);
  winner_sound();
  setLEDs(CHOICE_RED | CHOICE_YELLOW);
  winner_sound();
  setRGBLed(0, 0, 0); // Turn off RGB LED
  digitalWrite(buzzer, LOW);
}

void setRGBLed(int redValue, int greenValue, int blueValue)
{
  analogWrite(rgbLedRed, redValue);
  analogWrite(rgbLedGreen, greenValue);
  analogWrite(rgbLedBlue, blueValue);
}

// Play the winner sound
// This is just a unique (annoying) sound we came up with, there is no magic to it
void winner_sound(void)
{
  // Toggle the buzzer at various speeds
  for (byte x = 250 ; x > 70 ; x--)
  {
    for (byte y = 0 ; y < 3 ; y++)
    {
      digitalWrite(buzzer, LOW);
      delayMicroseconds(x);
      digitalWrite(buzzer, HIGH);
      delayMicroseconds(x);
    }
  }
}

// Play the loser sound/lights
void play_loser(void)
{
  setRGBLed(255, 0, 0); // Set RGB LED to red
  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_YELLOW);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_RED | CHOICE_GREEN);
  buzz_sound(255, 1500);

  setLEDs(CHOICE_BLUE | CHOICE_YELLOW);
  buzz_sound(255, 1500);
  setRGBLed(0, 0, 0); // Turn off RGB LED
  digitalWrite(buzzer, LOW);
}

// Show an "attract mode" display while waiting for user to press button.
void attractMode(void)
{
  while(1) 
  {
    setLEDs(CHOICE_RED);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_BLUE);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_GREEN);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;

    setLEDs(CHOICE_YELLOW);
    delay(100);
    if (checkButton() != CHOICE_NONE) return;
  }
}


