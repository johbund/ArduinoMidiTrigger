// ArduinoMidiTrigger; Sends midi events when piezo elements sense a hit.
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


// ############################################################## midi constants
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


// ########################################################### project constants
#define SENSORS 4   // number of trigger pads
#define AVERAGE 5   // number of last measurements to average over

// analog input pins
const int sensorpins[SENSORS] = {A0, A1, A2, A3};

// threshold for sensor readings
const int thresholds[SENSORS] = {25, 100, 100, 80};

// clip sensor value at corresponding value
const int clipValues[SENSORS] = {800, 800, 800, 800};

// time until next note can be fired (in millis)
const int cooldowns[SENSORS] = {50, 50, 50, 50};

// pad assignment
const int notes[SENSORS] = {DB_CLHAT, DB_OPHAT, DB_CLAP, DB_KICK2};


// ################################################################### variables
int sensorValues[SENSORS][AVERAGE];   // array of last sensor measurements
unsigned int writePos;                // write position sensor measurement

float currValues[SENSORS];            // current sensor measurument
float prevValues[SENSORS];            // previous sensor measurument

bool noteReady[SENSORS];              // next note can be triggered

unsigned long lasttime[SENSORS];      // save time of last note trigger


// ####################################################################### setup
void setup() {
  int sens, i;

  // initialize variables
  for (sens = 0; sens < SENSORS; sens++) {
    for (i = 0; i < AVERAGE; i++)
      sensorValues[sens][i] = 0;
    writePos = 0;
    currValues[sens] = 0.0;
    prevValues[sens] = 0.0;
    lasttime[sens] = 0;
    noteReady[sens] = false;
  }
  
  Serial.begin(MIDI_BAUD);
}


// ######################################################################## loop
void loop() {
  int sens, i;

  // read all sensors and write to array
  for (sens = 0; sens < SENSORS; sens++) {
    sensorValues[sens][writePos] = analogRead(sensorpins[sens]);
  }
  writePos = (writePos + 1) % AVERAGE; 

  // compute the average of the measurements
  float sums[SENSORS];
  for (sens = 0; sens < SENSORS; sens++) {
    sums[sens] = 0;
    for (i = 0; i < AVERAGE; i++) {
      sums[sens] += sensorValues[sens][i];
    }
    currValues[sens] = sums[sens] / (float) AVERAGE;
  }

  // fire Midi note on peak
  for (sens = 0; sens < SENSORS; sens++) {
    if (currValues[sens] > thresholds[sens]) {
      if (currValues[sens] >= prevValues[sens]) {
        noteReady[sens] = true;
      } else {
        if (noteReady[sens]) {
          if (lasttime[sens] + cooldowns[sens] < millis()) { 
            noteOn(notes[sens], velocityMap(prevValues[sens], sens)); //note_on
            noteOff(notes[sens]);                                     //note_off

            lasttime[sens] = millis();
            noteReady[sens] = false;
          }
        }
      }
      prevValues[sens] = currValues[sens];
    }
  }
}


// ####################################################### function declarations

// map sensor value on range 0 to clipValue
int velocityMap(float value, int sens) {
  if (value >= clipValues[sens]) {
    return 127;
  }
  float percent = value / (float) clipValues[sens];
  return (int) (percent * 127 + 0.5); 
}

void noteOn(int pitch, int velocity) {
  int cmd = MIDI_NOTE_ON + MIDI_CHANNEL - 1;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void noteOff(int pitch) {
  int cmd = MIDI_NOTE_OFF + MIDI_CHANNEL - 1;
  int velocity = 0x00;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
