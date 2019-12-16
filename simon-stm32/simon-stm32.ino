/**
  Simon Game for STM32F030

  Copyright (C) 2016-2019, Uri Shaked

  Released under the MIT License.
*/

#include "pitches.h"

/* Constants - define pin numbers for leds, buttons and speaker, and also the game tones */

// These are the proposed pin assignments for STM32F030:
char ledPins[] = { PA0, PA1, PA2, PA3 };
char buttonPins[] = { PA4, PA5, PA6, PA7 };
#define SPEAKER_PIN PB1

// These are connected to 74HC595 shift register (used to show game score):
int latchPin = PF1;  // 74HC595 pin 12
int dataPin = PA10;   // 74HC595pin 14
int clockPin = PF0;  // 74HC595 pin 11

#define MAX_GAME_LENGTH 100

int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5 };

/* Global variales - store the game state */

byte gameSequence[MAX_GAME_LENGTH] = {0};
byte gameIndex = 0;

uint8_t digitTable[] = {
  0b11000000,
  0b11111001,
  0b10100100,
  0b10110000,
  0b10011001,
  0b10010010,
  0b10000010,
  0b11111000,
  0b10000000,
  0b10010000,
};

void sendScore(uint8_t high, uint8_t low) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, low);
  shiftOut(dataPin, clockPin, MSBFIRST, high);
  digitalWrite(latchPin, HIGH);
}

void displayScore() {
  int high = gameIndex % 100 / 10;
  int low = gameIndex % 10;
  sendScore(high ? digitTable[high] : 0xff, digitTable[low]);
}

/**
  Slow down the clock - use HSI (High Speed Internal) clock,
  without PLL and with a divider of 4, giving us a 2MHZ speed.

  This helps us reduce the power usage, making it possible to
  power the game from a single CR2032 battery.
*/
void SystemClock_Config()
{
  // Avg power consumption with RCC_SYSCLKSOURCE_HSI when game starts: 6.6ma
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    _Error_Handler(__FILE__,  __LINE__);
  }

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }
}

/**
  Configure all the game pins
*/
void setup() {
  // The following line primes the random number generator. It assumes pin PA6 is floating (disconnected).
  randomSeed(analogRead(PA6));

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  pinMode(SPEAKER_PIN, OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // If the first button is pressed, run in Demo Mode
  if (!digitalRead(buttonPins[0])) {
    gameIndex = 0;
    pinMode(SPEAKER_PIN, INPUT);
    while (1) {
      tone(SPEAKER_PIN, gameTones[gameIndex % 4]);
      digitalWrite(ledPins[gameIndex % 4], HIGH);
      delay(100);
      digitalWrite(ledPins[gameIndex % 4], LOW);
      noTone(SPEAKER_PIN);
      displayScore();
      gameIndex += 1;
      delay(500);
    }
  }
}

/**
  Lights the given led and plays the suitable tone
*/
void lightLedAndPlaySound(byte ledIndex) {
  digitalWrite(ledPins[ledIndex], HIGH);
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  digitalWrite(ledPins[ledIndex], LOW);
  noTone(SPEAKER_PIN);
}

/**
  Plays the current sequence of notes that the user has to repeat
*/
void playSequence() {
  for (int i = 0; i < gameIndex; i++) {
    char currentLed = gameSequence[i];
    lightLedAndPlaySound(currentLed);
    delay(50);
  }
}

/**
  Waits until the user pressed one of the buttons, and returns the index of that button
*/
byte readButton() {
  for (;;) {
    for (int i = 0; i < 4; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}

/**
  Play the game over sequence, and report the game score
*/
void gameOver() {
  gameIndex = 0;
  delay(200);
  // Play a Wah-Wah-Wah-Wah sound
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (int i = 0; i < 200; i++) {
    tone(SPEAKER_PIN, NOTE_C5 + (i % 20 - 10));
    delay(5);
  }
  noTone(SPEAKER_PIN);

  displayScore();
  sendScore(~0x40, ~0x40);
  delay(200);
}

/**
  Get the user input and compare it with the expected sequence.
  If the user fails, play the game over sequence and restart the game.
*/
void checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    char expectedButton = gameSequence[i];
    char actualButton = readButton();
    lightLedAndPlaySound(actualButton);
    if (expectedButton == actualButton) {
      /* good */
    } else {
      gameOver();
      return;
    }
  }
}

/**
  Plays an hooray sound whenever the user finishes a level
*/
void levelUp() {
  tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);
}

/**
  The main game loop
*/
void loop() {
  displayScore();

  // Add a random color to the end of the sequence
  gameSequence[gameIndex] = random(0, 4);
  gameIndex++;

  playSequence();
  checkUserSequence();
  delay(300);

  if (gameIndex > 0) {
    levelUp();
    delay(300);
  }
}
