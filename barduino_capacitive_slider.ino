#define N_ELECTRODES 8

unsigned int PINS[N_ELECTRODES] = { 1, 2, 4, 5, 6, 12, 13, 14 };


void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < N_ELECTRODES; i++) {
    Serial.print(touchRead(PINS[i]));
    Serial.print(",");
  }
  Serial.println("");


  delay(10);
}
