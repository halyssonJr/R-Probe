#include "common.h"
#include "board.h"
#include "esp_camera.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "camera.h"

#define TAG "[CAMERA]"

esp_err_t camera_init(void)
{
   esp_err_t err = ESP_OK;

   gpio_config_t conf = {
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
      .pin_bit_mask = 1LL << 13,
   };

   gpio_config(&conf);

   camera_config_t config = {
      .ledc_channel = LEDC_CHANNEL_0,
      .ledc_timer = LEDC_TIMER_0,
      .pin_d0 = Y2_GPIO_NUM,
      .pin_d1 = Y3_GPIO_NUM,
      .pin_d2 = Y4_GPIO_NUM,
      .pin_d3 = Y5_GPIO_NUM,
      .pin_d4 = Y6_GPIO_NUM,
      .pin_d5 = Y7_GPIO_NUM,
      .pin_d6 = Y8_GPIO_NUM,
      .pin_d7 = Y9_GPIO_NUM,
      .pin_xclk = XCLK_GPIO_NUM,
      .pin_pclk = PCLK_GPIO_NUM,
      .pin_vsync = VSYNC_GPIO_NUM,
      .pin_href = HREF_GPIO_NUM,
      .pin_sscb_sda = SIOD_GPIO_NUM,
      .pin_sscb_scl = SIOC_GPIO_NUM,
      .pin_reset = RESET_GPIO_NUM,
      .pin_pwdn = PWDN_GPIO_NUM,
      .xclk_freq_hz = XCLK_FREQ,
      .pixel_format = CAMERA_PIXEL_FORMAT,
      .frame_size = CAMERA_FRAME_SIZE,
      .jpeg_quality = 10,
      .fb_count = 2,
   };

   err = esp_camera_init(&config);
   if (err != ESP_OK) 
   {
      ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
      return err;
   }

   sensor_t *s = esp_camera_sensor_get();
   s->set_framesize(s, CAMERA_FRAME_SIZE);

   return err;
}
