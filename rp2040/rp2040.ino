// Raspberry PI RP2040
// Adaptive Sampling

// Shreya Ashok

#include <Arduino.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// UART : GPIO 16 - TX , GPIO 17 - RX

// Audio classification
#include <MicroTFLite.h>
#include "sound_model.h"
#include "preprocessing.h"

constexpr int kTensorArenaSize = 16 * 1024;
alignas(16) byte tensor_arena[kTensorArenaSize];

const char* classes[] = {"clap", "music","noise", "speech"};

const float INPUT_SCALE = 0.03461882099509239f;
const int INPUT_ZERO_POINT = -23;

// processing feature vectors

char lineBuffer[512];
size_t lineLength = 0;

void processFeatureLine(char* line){
  float features[28];
  int count = 0;

  char* token = strtok(line, ",");

  while (token!=nullptr && count < 28){
    char* endptr;
    features[count] = strtof(token, &endptr);

    if (endptr==token || *endptr != '\0'){
      Serial.println("Invalid no.");
      return;
    }
    count++;
    token = strtok(nullptr, ",");
  }
  if (count!=28 || token != nullptr){
    Serial.print("Invalid feature count: ");
    Serial.println(count);
    return;
  }
  // 1. Print raw features received from the ESP32
  Serial.println("\n========== RECEIVED FEATURES ==========");

  for (int i = 0; i < 28; i++) {
    Serial.print("F[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(features[i], 6);
  }
  // 2. Standardise features and send to the model
  float standardised[28];

  Serial.println("========== STANDARDISED FEATURES ==========");

  for (int i = 0; i < 28; i++) {
    if (!isfinite(feature_scale[i]) || feature_scale[i] == 0.0f) {
      Serial.print("Invalid scaler value at index ");
      Serial.println(i);
      return;
    }

    standardised[i] =
        (features[i] - feature_mean[i]) / feature_scale[i];

    if (!isfinite(standardised[i])) {
      Serial.print("Invalid standardised feature at index ");
      Serial.println(i);
      return;
    }

    Serial.print("Z[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(standardised[i], 6);

    if (!ModelSetInput(standardised[i], i, true)) { {
      Serial.print("Failed to set model input at index ");
      Serial.println(i);
      return;
    }
    }
  }
  unsigned long t0 = micros();

  if (!ModelRunInference()) {
    Serial.println("Inference failed.");
    return;
  }
  Serial.print("Inference took ");
  Serial.print(micros() - t0);
  Serial.println(" us");
  int bestIndex = 0;
  float bestScore = ModelGetOutput(0);

  Serial.print("Scores: ");

  for (int i = 0; i < 4; i++) {
    float score = ModelGetOutput(i);

    Serial.print(classes[i]);
    Serial.print("=");
    Serial.print(score, 4);

    if (i < 3) Serial.print(", ");

    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  Serial.print("\nPredicted class: ");
  Serial.println(classes[bestIndex]);
  Serial.println("--------------------");
}
// UART 
int currentRate;
float signalLevel;

// Parameters: (TX Pin, RX Pin, RTS Pin, CTS Pin)
arduino::UART Serial2(4, 5, NC, NC);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  while(!Serial) yield();

  if (!ModelInit(sound_classifier_int8_tflite, tensor_arena, kTensorArenaSize)) {
    Serial.println("Model initialisation failed.");
    while (1) yield();
  }

  Serial.println("Model loaded.");
  Serial.println("Input type and shape check:");

  ModelPrintTensorInfo();
  ModelPrintTensorQuantizationParams();

  // UART
  while (!Serial){
    ;
  }
  Serial.println("Initialised. Receiving data from UART");
}

void loop() {
  while (Serial2.available()) {
    char c = (char)Serial2.read();

    if (c == '\r') continue;

    if (c == '\n') {
      if (lineLength > 0) {
        lineBuffer[lineLength] = '\0';
        processFeatureLine(lineBuffer);
        lineLength = 0;
      }
    } else if (lineLength < sizeof(lineBuffer) - 1) {
      lineBuffer[lineLength++] = c;
    } else {
      // Discard the overflowing line.
      lineLength = 0;
      while (Serial2.available()) {
        if (Serial2.read() == '\n') break;
      }
      Serial.println("UART line too long; discarded.");
    }
  }
}
