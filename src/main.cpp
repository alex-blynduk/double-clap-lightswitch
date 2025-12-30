#include <WiFi.h>

#define DOUBLE_CLAP_WINDOW 800 // Max gap between two claps (ms)
#define MIC_PIN 34           // Microphone OUT connected here
#define LED_PIN 25           // Controls transistor base
#define CLAP_DELAY 500       // Minimum time between claps (ms)
#define FREQ_TIMEGAP 100     // in miliseconds

#define sign(x) ((x) >= 0 ? 1 : -1)

bool ledState = false;
unsigned long lastClapTime = 0;
static bool soundActive = false;
static unsigned long soundStart = 0;
unsigned long dur = 0;
unsigned long firstClapTime = 0;
int clapCount = 0;


bool detectClap() {
  // int prevAmp = analogRead(MIC_PIN) - 2048;  // ESP32 ADC is 0–4095
  // int zeroCrossings = 0;
  // int highAmp = 0;

  // // ~80 samples over about 10 ms
  // for (int i = 0; i < 55; i++) {
  //   int amp = analogRead(MIC_PIN) - 2048;
  //   if (sign(amp) != sign(prevAmp)) zeroCrossings++;
  //   if (abs(amp) > 1500) highAmp++;
  //   prevAmp = amp;
  //   delayMicroseconds(200);  // tune sampling rate (~5 kHz)
  // }

  // bool clap = (highAmp > 45 && zeroCrossings > 20);
  // if (clap) {
  //   Serial.print("Zero crossings: ");
  //   Serial.print(zeroCrossings);
  //   Serial.print(" | High amp count: ");
  //   Serial.print(highAmp);
  //   Serial.print(" | amplitude: ");
  //   Serial.println(prevAmp);
  // }
  // return clap;

  int zeroCrossings = 0;
  int prev = analogRead(MIC_PIN);
  long highAmp = 0;
  unsigned long start = millis();
  
  while (millis() - start < FREQ_TIMEGAP) {
    int current = analogRead(MIC_PIN);
    if (abs(current) > 4000) {
      highAmp++;

      if (sign(current - 2048) != sign(prev - 2048)) {
        zeroCrossings++;
      }
    }

    prev = current;
  }

  float freq = (zeroCrossings / 2.0) / ((float)((float)FREQ_TIMEGAP / 1000.0)); //turn miliseconds into seconds

  //Simple loudness threshold to measure "active" duration
  if (highAmp > 120 && !soundActive) {
    soundActive = true;
    soundStart = millis();
  }
  if (highAmp < 120 && soundActive) {
    soundActive = false;
    dur = millis() - soundStart;
  }

  if (highAmp > 150 && highAmp < 400 && freq > 300 && dur > 30 && dur < 150) {
    Serial.print("Sound lasted ");
    Serial.print(dur);
    Serial.println(" ms");
    Serial.print(" | Amplitude: ");
    Serial.print(highAmp);
    Serial.print(" | Frequency: ");
    Serial.print(freq);
    Serial.println(" Hz");

    return true;
  }

  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("=== Clap Detector ===");
}

void loop() {

    Serial.println(analogRead(MIC_PIN));


  if (detectClap()) {
    unsigned long now = millis();
    Serial.println("entered");

    if (clapCount == 0) {
      clapCount = 1;
      firstClapTime = now;
      Serial.println("First clap detected, waiting for second...");
    } 
    else if (clapCount == 1 && (now - firstClapTime) < DOUBLE_CLAP_WINDOW) {
      clapCount = 0;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);

      Serial.println("---- DOUBLE CLAP DETECTED ----");
      Serial.print("LED: ");
      Serial.println(ledState ? "ON" : "OFF");
      Serial.println("------------------------------");
    }
  }

  // If time window expires, reset counter
  if (clapCount == 1 && (millis() - firstClapTime > DOUBLE_CLAP_WINDOW)) {
    clapCount = 0;
  }

  delay(20);
}


