#define SENSOR_PIN A3

#define THD 25
#define MAX_VALUE 800

#define AVG 5

#define COOLDOWN 50

int sensVal_avg;
int sensVal_old;
int sensVal_old_old;

unsigned long lasttime;

int i;

float sensVal[AVG];

bool note_ready;

void setup() {
  Serial.begin(31250);

  sensVal_avg = 0;
  sensVal_old = 0;
  sensVal_old_old = 0;

  lasttime = 0;

  for (i = 0; i < AVG; i++) {
    sensVal[i] = 0;
  }
  
  note_ready = false;
}

void loop() {

  for (i = 1; i < AVG; i++) {
    sensVal[i] = sensVal[i-1];
  }
  sensVal[0] = analogRead(SENSOR_PIN);

  double sum = 0;
  for (i = 0; i < AVG; i++) {
    sum += sensVal[i];
  }
  sensVal_avg = (int) (sum / (float) AVG);
  
  if (sensVal_avg > THD) {

//  Serial.print(read_sens);
//  Serial.print(", ");
  Serial.print(sensVal_avg);
//  Serial.print(", ");
//  Serial.print((int) (((ppSensVal + pSensVal + sensVal) / 3.0));
  Serial.println();
    
//    Serial.println(sensorVal);
    if (sensVal_avg >= sensVal_old) {
      note_ready = true;
    } 

    if (sensVal_avg < sensVal_old && sensVal_old < sensVal_old_old) {
      if (note_ready) {
        if (lasttime + COOLDOWN < millis()) {
        
        noteOn(0x99, 0x27, velocityMap(sensVal_old_old)); //note_on
        noteOn(0x89, 0x27, velocityMap(sensVal_old)); //note_off

        lasttime = millis();
        note_ready = false;
        }
      }
    }

  sensVal_old_old = sensVal_old;
  sensVal_old = sensVal_avg;
  }
}

int velocityMap(int value) {
  if (value > MAX_VALUE) {
    return 127;
  }
  float percent = (float) value / (float) MAX_VALUE;
//  Serial.println(percent);
  return (int) (percent * 127); 
}

void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
//  Serial.print("Midi");
}
