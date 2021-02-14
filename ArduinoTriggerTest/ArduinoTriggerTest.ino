// ArduinoTriggerTest; Test a piezo drum pad.
// Copyright (C) 2021  Johannes Bund
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// ##################################################### preliminary definitions
#define MIDI_NOTE_ON    0x90
#define MIDI_NOTE_OFF   0x80

#define MIDI_BAUD       31250
#define SERIAL_BAUD     9600

// Midi channel (Arturia DrumBrute)
#define MIDI_CHANNEL    10

// Arturia DrumBrute Midi map
#define DB_KICK1        0x24
#define DB_KICK2        0x25
#define DB_SNARE        0x26
#define DB_CLAP         0x27
#define DB_RIM          0x28
#define DB_CLAVES       0x29
#define DB_CLHAT        0x2a
#define DB_OPHAT        0x2b
#define DB_CONGAH       0x2c
#define DB_TOMH         0x2d
#define DB_CONGAL       0x2e
#define DB_TOML         0x2f
#define DB_CYMBAL       0x30
#define DB_REVCYMBAL    0x31
#define DB_MARACAS      0x32
#define DB_TAMB         0x33
#define DB_ZAP          0x34


// ############################################# adjust the following parameters
#define SENSOR_PIN A0

#define AVERAGE 5           // number of last measurements to average over

#define THRESHOLD 25        // threshold for sensor readings

#define MAX_VALUE 800       // clip sensor values at MAX_VALUE

#define COOLDOWN 50         // time until next note can be fired (in millis)


int sensorValues[AVERAGE];  // array of last sensor measurements
unsigned int writePos;      // write position of next sensor measurement

float currValue;            // current average over sensor measuruments 
float prevValue;            // previous average over sensor measuruments 

bool noteReady;             // indicates that next note can be triggered

unsigned long lasttime;     // save time of last note trigger


// ####################################################################### setup
void setup() {
  int i;

  // initialize variables
  for (i = 0; i < AVERAGE; i++) {
    sensorValues[i] = 0;
  }
  writePos = 0;
  currValue = 0.0;
  prevValue = 0.0;
  lasttime = 0;
  noteReady = false;
  
  Serial.begin(SERIAL_BAUD);
}


// ######################################################################## loop
void loop() {
  int i;
  char output[32];

  writePos = (writePos + 1) % AVERAGE;              // advance write position
  sensorValues[writePos] = analogRead(SENSOR_PIN);  // read sensor

  unsigned int sum = 0;                             // compute average
  for (i = 0; i < AVERAGE; i++) {
    sum += sensorValues[i];
  }
  currValue = sum / (float) AVERAGE;
  
  if (currValue > THRESHOLD) {
    
    sprintf(output, "  sensor: %d, average: %d.%02d", sensorValues[writePos], 
                             (int)currValue, (unsigned int)(currValue*100)%100);
    Serial.println(output);
    
    if (currValue >= prevValue) {
      noteReady = true;
    } else {
      if (noteReady) {
        if (lasttime + COOLDOWN < millis()) {
        
        noteOn(DB_CLAP, velocityMap(prevValue));  //note_on
        noteOff(DB_CLAP);                         //note_off

        lasttime = millis();
        noteReady = false;
        }
      }
    }

  prevValue = currValue;
  }
}


// ####################################################### function declarations

// map sensor value to range 0 to MAX_VALUE
int velocityMap(float value) {
  char output[32];
  int result;

  if (value >= MAX_VALUE) {
    result = 127;
  }
  float percent = value / (float) MAX_VALUE;
  result = (int) (percent * 127 + 0.5);

  sprintf(output, "  velocity map %d.%02d to %d", (int)value, 
                            (unsigned int)(value*100)%100, result);
  Serial.println(output);

  return result;
}

void noteOn(int pitch, int velocity) {
  char output[32];
  sprintf(output, "MIDI note %x on, velocity: %d", pitch, velocity);
  Serial.println(output);

/*  // change baud rate when sending Midi signals
  int cmd = MIDI_NOTE_ON + MIDI_CHANNEL - 1;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
*/
}

void noteOff(int pitch) {
  char output[32];
  sprintf(output, "MIDI note %x off", pitch);
  Serial.println(output);
  
/*  // change baud rate when sending Midi signals
  int cmd = MIDI_NOTE_OFF + MIDI_CHANNEL - 1;
  int velocity = 0x00;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
*/
}
