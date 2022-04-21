// An Arduino-powered Pomodoro clock
// serial print statements for debugging only
#include "pitches.h"

int counter = 0;

// pins
const int switchPin = 2;
int switchState = 0;

// RGB
const int greenLEDPin = 9;
const int blueLEDPin = 10;
const int redLEDPin = 11;

// counter LED
const int redCountLED = 8;
const int yellowCountLED = 7;
const int greenCountLED = 6;

// piezo pin
const int piezoPin = 3;

// work interval
long workInterval = 900000;

// long break
long longBreak = 600000;

// short break
long shortBreak = 120000;

// store last time timer was reset
unsigned long previousTime = 0;

// initialize blue LED
int brightBlue = 0;

// melodies
int melody1[] = {
  NOTE_C3, NOTE_E3, NOTE_G3, NOTE_C4
};

int durs1[] = {
  4, 4, 4, 2
};

int melody2[] = {
  NOTE_C4, NOTE_G3, NOTE_E3, NOTE_C3
};

int melody3[] = {
  NOTE_C3, NOTE_F3, NOTE_G3,
  NOTE_F3, NOTE_G3, NOTE_C4,
  NOTE_G3, NOTE_C4, NOTE_F4,
  NOTE_C4, NOTE_F4, NOTE_G4
};

int durs3[] = {
  8, 8, 8,
  8, 8, 8,
  8, 8, 8,
  8, 8, 2
};

int melody4[] = {
  NOTE_G4, NOTE_F4, NOTE_C4,
  NOTE_F4, NOTE_C4, NOTE_G3,
  NOTE_C4, NOTE_G3, NOTE_F3,
  NOTE_G3, NOTE_F3, NOTE_C3
};

void setup() {
  // put your setup code here, to run once:
  // serial port
  Serial.begin(9600);

  // set the RGB LED pin modes
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);

  // set pin modes for the counter LEDs:
  pinMode(redCountLED, OUTPUT);
  pinMode(yellowCountLED, OUTPUT);
  pinMode(greenCountLED, OUTPUT);

  // set pin mode for piezo pin
  pinMode(piezoPin, OUTPUT);

  // set button pin mode:
  pinMode(switchPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // switch: sets counter +1, switches between work and break, switches to long break, resets counter to 0
  switchState = digitalRead(switchPin);
  // counter: which part of the pomodoro cycle are we on?
  // parts of the cycle: work 1, break 1, work 2, break 2, work 3, longbreak
  // 0 = start
  // 1 = work 1
  // 2 = break 1
  // 3 = work 2
  // 4 = break 2
  // 5 = work 3
  // 6 = longbreak
  if (switchState == HIGH) {
    counter++;
  }
  if (counter > 6) {
    counter = 0;
  }
  Serial.println(counter);

  // write the counter LED
  if ( counter == 1 ) {
    digitalWrite(redCountLED, HIGH);
  }
  else if ( counter == 3 ) {
    digitalWrite(yellowCountLED, HIGH);
  }
  else if ( counter == 5 ) {
    digitalWrite(greenCountLED, HIGH);
  }
  else if (counter == 0) {
    digitalWrite(redCountLED, LOW);
    digitalWrite(yellowCountLED, LOW);
    digitalWrite(greenCountLED, LOW);
    analogWrite(blueLEDPin, 0);
  }

  // timer: measures time since last change of state
  unsigned long currentTime = millis();
  float diff = currentTime - previousTime;

  // break
  if (counter % 2 == 0 && counter > 0) {
    analogWrite(redLEDPin, 0); // turn red part of the RGB LED off
    analogWrite(greenLEDPin, 0); // turn gree part of the RGB LED off

    unsigned long quotient = ((unsigned long)diff) / 20000;
    float remainder = diff - quotient * 20000;
    float frac2 = remainder / 20000;

    if (frac2 > 0 && frac2 < 0.5) {
      brightBlue = 512 * frac2;
      analogWrite(blueLEDPin, brightBlue);
    }
    else if (frac2 < 1 && frac2 > 0.5) {
      brightBlue = 512 * (1 - frac2);
      analogWrite(blueLEDPin, brightBlue);
    }
    else {
      Serial.print("value out of range!");
      brightBlue = 0;
      analogWrite(blueLEDPin, brightBlue);
      }

    if ( counter < 5 && diff > shortBreak ) {
      previousTime = currentTime;

      // play short break melody
      for (int note = 0; note < 4; note++) {
        int noteDur = 1000 / durs1[note];

        tone(piezoPin, melody2[note], noteDur);

        // pause between notes
        int pause = noteDur * 1.25;
        delay(pause);

        noTone(piezoPin);
      }
    }
    else if ( counter == 6 && diff > longBreak) {
      previousTime = currentTime;
      // play long break end melody
      for (int note = 0; note < 12; note++) {
        int noteDur = 1000 / durs3[note];

        tone(piezoPin, melody4[note], noteDur);

        // pause between notes
        int pause = noteDur * 1.25;
        delay(pause);

        noTone(piezoPin);
      }
    }
  }

  // work led: fading from red to green
  // red: begins brightly, stays for half of the time, fades out
  // green: begins at 0, fades in, reaches max at half of time
  // to make yellow, the curves must overlap
  // workInterval/2 marks the point where red starts fading out and green has attained its full value
  // output range: 0-255
  float frac = diff / workInterval; // which ratio of the worktime is used up?
  float revfrac = 1 - frac;
  int brightRed = 255;
  int brightGreen = 0;

  if (counter % 2 != 0 && diff <= workInterval) {
    brightBlue = 0;
    analogWrite(blueLEDPin, brightBlue);
    if ( frac >= 0 && frac < 0.5) {
      brightGreen = 512 * frac;
      analogWrite(redLEDPin, 255);
      analogWrite(greenLEDPin, brightGreen);
    }
    else if (frac >= 0.5 && frac <= 1) {
      analogWrite(greenLEDPin, 255);
      brightRed = 512 * revfrac;
      analogWrite(redLEDPin, brightRed);
    }
  }
  else if (counter % 2 != 0 && diff > workInterval) {
    previousTime = currentTime;

    if ( counter < 5 ) {
      //play short break melody
      for (int note = 0; note < 4; note++) {
        int noteDur = 1000 / durs1[note];

        tone(piezoPin, melody1[note], noteDur);

        // pause between notes
        int pause = noteDur * 1.25;
        delay(pause);

        noTone(piezoPin);
      }
    }
    else if (counter == 5) {
      // play long break melody
      for (int note = 0; note < 12; note++) {
        int noteDur = 1000 / durs3[note];

        tone(piezoPin, melody3[note], noteDur);

        // pause between notes
        int pause = noteDur * 1.25;
        delay(pause);

        noTone(piezoPin);
      }
    }
  }

  delay(200);
}
