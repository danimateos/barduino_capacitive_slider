#define N_ELECTRODES 8
#define N_SAMPLES 10

unsigned int PINS[N_ELECTRODES] = { 1, 2, 4, 5, 6, 12, 13, 14 };
uint32_t baseline[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };

uint32_t buffer[N_ELECTRODES][N_SAMPLES] = { 0 };

void setup() {
  Serial.begin(115200);

  recordBaseline();
}

void loop() {
  for (int e = 0; e < N_ELECTRODES; e++) {
    Serial.print(touchRead(PINS[e]));
    Serial.print(",");
  }
  Serial.println("");


  delay(10);
}

void recordBaseline() {
  for (int s = 0; s < N_SAMPLES; s++) {
    for (int e = 0; e < N_ELECTRODES; e++) {
      buffer[e][s] = touchRead(PINS[e]);
    }
  }
}

void aggregate() {
  for (int i = 0; i < sizeof(buffer) / sizeof(bufferx[0]); i++) {}
}