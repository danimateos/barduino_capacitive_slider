#define N_ELECTRODES 8
#define N_SAMPLES 10
#define WAIT_SAMPLES 500
#define MIN_READS 300

#define SAFE_SUB(a, b) ((a) >= (b) ? (a) - (b) : 0)

unsigned int PINS[N_ELECTRODES] = { 1, 2, 4, 5, 6, 12, 13, 14 };

uint32_t sampleBuffer[N_ELECTRODES][N_SAMPLES] = { 0 };
uint32_t baseline[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t averages[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t normalized[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };

uint32_t sampleIndex = 0;

float position = 0.0;

void setup() {
  Serial.begin(115200);


  Serial.println("Recording baseline");
  recordFromScratch();
  waitUntilStable();

  Serial.println("Aggregating baseline");
  aggregateInto(baseline);

  printArray(baseline, N_ELECTRODES);
}

void loop() {
  readOne();
  aggregateInto(averages);
  subtract(normalized, averages, baseline);
  Serial.print(position);
  Serial.print(",");
  printArray(normalized, N_ELECTRODES);
  updatePosition();
  delay(10);
}

void recordFromScratch() {
  for (int s = 0; s < N_SAMPLES; s++) {
    for (int e = 0; e < N_ELECTRODES; e++) {
      int pin = PINS[e];
      uint32_t reading = touchRead(pin);
      // Serial.print("Reading pin ");
      // Serial.print(pin);
      // Serial.print(" for a value of ");
      // Serial.println(reading);
      // sampleBuffer[e][s] = reading;
    }
  }
}

void aggregateInto(uint32_t* out) {
  for (int e = 0; e < N_ELECTRODES; e++) {
    uint64_t partial = 0;
    for (int s = 0; s < N_SAMPLES; s++) {
      partial += sampleBuffer[e][s];
    }
    out[e] = partial / N_SAMPLES;
  }
}

void printArray(uint32_t* array, int len) {
  for (int i = 0; i < len; i++) {
    Serial.print(array[i]);
    Serial.print(",");
  }
  Serial.println("");
}

void readOne() {
  for (int e = 0; e < N_ELECTRODES; e++) {
    sampleBuffer[e][sampleIndex % N_SAMPLES] = touchRead(PINS[e]);
  }
  sampleIndex++;
}

void subtract(uint32_t* out, uint32_t* averages, uint32_t* baseline) {

  for (int e = 0; e < N_ELECTRODES; e++) {

    out[e] = SAFE_SUB(averages[e], baseline[e]);
  }
}

void waitUntilStable() {
  while (sampleIndex < WAIT_SAMPLES) {
    readOne();
    delay(1);
  }
}

void updatePosition() {
  float total = 0.0;
  float weighted = 0.0;


  for (int e = 0; e < N_ELECTRODES; e++) {
    total += (float)normalized[e];
    weighted += (e + 1) * (float)normalized[e];
  }

  position = total < MIN_READS ? 0.5 : ((weighted / total) - 1) / (N_ELECTRODES - 1);
}