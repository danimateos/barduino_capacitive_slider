#include <Adafruit_NeoPixel.h>

#define N_ELECTRODES 8
#define N_SAMPLES 10
#define WAIT_SAMPLES 500
#define MIN_READS 300
#define N_PIXELS 14

#define SAFE_SUB(a, b) ((a) >= (b) ? (a) - (b) : 0)

unsigned int PINS[N_ELECTRODES] = { 1, 2, 4, 5, 6, 12, 13, 14 };
const int LED_PIN = 11;
int MAX_READING = 10000;  // used to calibrate the Neopixels' brightness

uint32_t sampleBuffer[N_ELECTRODES][N_SAMPLES] = { 0 };
uint32_t baseline[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t averages[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t normalized[N_ELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };



uint32_t sampleIndex = 0;

float position = 0.0;

Adafruit_NeoPixel strip(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();

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

  display1DArray(normalized, strip.Color(155, 0, 155));
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

// Show the readings of the capacitive sensors as brightness values in the array
// Assumes same number of electrodes and pixels
void display1DArray(uint32_t* values, uint32_t color) {

  int r = (color >> 16) & 0xFF;
  int g = (color >> 8) & 0xFF;
  int b = (color >> 0) & 0xFF;



  for (int i = 0; i < N_ELECTRODES; i++) {
    float factor = (float)values[i] / MAX_READING;

    strip.setPixelColor(i, strip.Color((uint8_t)(factor * r), (uint8_t)(factor * g), (uint8_t)(factor * b)));
  }

  strip.show();
}


void distributeOnto(const int from[], const float overlapFractions[][N_PIXELS], int out[], int sizeFrom, int sizeTo) {
  // Clear
  for (int p = 0; p < sizeTo; p++) {
    out[p] = 0;
  }

  // Electrodes
  for (int e = 0; e < sizeFrom; e++) {
    // Pixels
    for (int p = 0; p < sizeTo; p++) {
      out[p] += (from[e] * overlapFractions[e][p]);
    }
  }
}

