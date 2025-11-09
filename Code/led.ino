#include <driver/i2s.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define NUM_LEDS 60

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// main config
#define I2S_SAMPLE_RATE   44100
#define I2S_READ_LEN      1024
#define I2S_PORT          I2S_NUM_0

int16_t sBuffer[I2S_READ_LEN];

void setupI2S() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false
  };

  // pins configs
  i2s_pin_config_t pin_config = {
    .bck_io_num = 26, 
    .ws_io_num  = 25, 
    .data_out_num = -1,
    .data_in_num  = 27
  };

  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);
  setupI2S();
  strip.begin();
  strip.show();
}

void loop() {
  size_t bytes_read = 0;
  i2s_read(I2S_PORT, (char*)sBuffer, I2S_READ_LEN, &bytes_read, portMAX_DELAY);

  long sum = 0;
  int samples = bytes_read / 4;

  for (int i = 0; i < samples; i++) {
    int32_t sample = sBuffer[i] >> 14;  
    sum += abs(sample);
  }

  long level = sum / samples;  
  level = constrain(level, 200, 4000);  

  int brightness = map(level, 200, 4000, 10, 255);
  brightness = constrain(brightness, 10, 255);

  uint32_t color = strip.Color(brightness, 0, 255 - brightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}