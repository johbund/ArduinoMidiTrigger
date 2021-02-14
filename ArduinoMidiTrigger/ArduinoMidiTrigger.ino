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

#define SENSORS 4
#define AVERAGE 5

int sensorpins[SENSORS] = {A0, A1, A2, A3};

int thresholds[SENSORS] = {25, 25, 25, 25};

int clip_values[SENSORS] = {800, 800, 800, 800};

int cooldowns[SENSORS] = {50, 50, 50, 50};

int notes[SENSORS] = {0x25, 0x26, 0x27, 0x28};


int sensorValues[SENSORS][AVERAGE];
int avgValues[SENSORS];
int avgValues_prev[SENSORS];
int avgValues_prev_prev[SENSORS];
unsigned long last_triggered[SENSORS];
bool note_ready[SENSORS];

int sens, i;

void setup() {
  Serial.begin(31250);

  for (sens = 0; sens < SENSORS; sens++) {
    for (i = 0; i < AVERAGE; i++)
      sensorValues[sens][i] = 0;
    avgValues[sens] = 0;
    avgValues_prev[sens] = 0;
    avgValues_prev_prev[sens] = 0;
    last_triggered[sens] = 0;
    note_ready[sens] = false;
  }
}

void loop() {

  // shift the previous values by one
  // TODO: ring buffer
  for (sens = 0; sens < SENSORS; sens++) {
    for (i = AVERAGE - 1; i > 0; i--) {
      sensorValues[sens][i] = sensorValues[sens][i-1];
    }
    sensorValues[sens][0] = analogRead(sensorpins[sens]);
  }

  // compute the average of the measurements
  float sums[SENSORS];
  for (sens = 0; sens < SENSORS; sens++) {
    avgValues_prev_prev[sens] = avgValues_prev[sens];
    avgValues_prev[sens]  = avgValues[sens];
    
    sums[sens] = 0;
    for (i = 0; i < AVERAGE; i++) {
      sums[sens] += sensorValues[sens][i];
    }
    avgValues[sens] = sums[sens] / AVERAGE;
  }

  for (sens = 0; sens < SENSORS; sens++) {
    if (avgValues[sens] > thresholds[sens]) {

      //on rising values wait for peak
      if (avgValues[sens] >= avgValues_prev[sens]) {
        note_ready[sens] = true;
      }

      if (avgValues_prev[sens] < avgValues_prev_prev[sens] && 
                              avgValues[sens] < avgValues_prev[sens]) {
        if (note_ready[sens]) {
          if (last_triggered[sens] + cooldowns[sens] < millis()) { 
            noteOn(0x99, notes[sens], velocityMap(avgValues_prev_prev[sens], sens)); //note_on
            noteOn(0x89, notes[sens], 0x00); //note_off

            last_triggered[sens] = millis();
            note_ready[sens] = false;
          }
        }
      }
    }
  }
}

int velocityMap(int value, int sens) {
  if (value > clip_values[sens]) {
    return 127;
  }
  float percent = (float) value / (float) clip_values[sens];
  return (int) (percent * 127); 
}

void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
//  Serial.print("Midi");
}
