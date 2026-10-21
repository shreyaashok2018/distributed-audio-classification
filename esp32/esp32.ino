// MIC + UART with RP2040

#define RXD2 16
#define TXD2 17

#include <Arduino.h>
#include <driver/i2s.h>
#include <ESP32Feature28.h>

static const i2s_port_t I2S_PORT = I2S_NUM_0;

static const int PIN_I2S_BCLK = 32;
static const int PIN_I2S_LRCK = 25;
static const int PIN_I2S_DOUT = 33;

// standard speech processing sampling rate 16000 Hz
static const uint32_t SAMPLE_RATE = 16000;

// recording duration for clear statistics 
static const uint32_t RECORD_SECONDS = 2;

static const uint32_t TOTAL_SAMPLES = SAMPLE_RATE * RECORD_SECONDS;

// chunk size for each i2s read
static const size_t CHUNK_SAMPLES= 512;

// Temporary buffer to read data
static int16_t i2sChunk[CHUNK_SAMPLES];

Feature28Extractor extractor (SAMPLE_RATE);

void setup_I2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags= ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk= 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = PIN_I2S_BCLK,
    .ws_io_num = PIN_I2S_LRCK,
    .data_out_num = -1,
    .data_in_num = PIN_I2S_DOUT
  };

  esp_err_t err;

  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK){
    Serial.printf("Error installing I2 driver :%d\n", err);
    return;
  }

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err!=ESP_OK){
    Serial.printf("Error setting I2S pins: %d\n", err);
    return;
  }

  i2s_zero_dma_buffer(I2S_PORT);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // UART2
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // I2S
  setup_I2S();

}
void loop(){
  uint32_t samplesReceived = 0;
  uint32_t lastProgress = 0;

  while (samplesReceived < TOTAL_SAMPLES){
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(
      I2S_PORT,
      (void*)i2sChunk,
      sizeof(i2sChunk),
      &bytesRead,
      portMAX_DELAY
    );
    if (result!= ESP_OK){
      Serial.printf("I2S read error: %d\n", result);
      continue;
    }
    size_t samplesRead = bytesRead / sizeof(int16_t);

    if (samplesRead==0){
      continue;
    }
    extractor.feed(i2sChunk, samplesRead);

    samplesReceived += samplesRead;

    uint32_t progress = (samplesReceived*100)/TOTAL_SAMPLES;
    if (progress>=lastProgress + 10){
      lastProgress = progress;
    }
  }
  float features[28];
  bool success = extractor.compute(features);

  if (!success) {
    Serial.println("Feature extraction failed");
    extractor.reset();
    return;
  }

  for (int i=0; i<28; i++){
  Serial.print(features[i]);
  if (i<27)
  Serial.print(",");
  }
  Serial.println();
  /* UART- disabled for data collection
  for (int i=0; i<28; i++){
  Serial2.print(features[i]);
  if (i<27)
  Serial2.print(",");
  }
  Serial2.println();
  */
  extractor.reset();

}