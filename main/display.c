#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"

#include "board.h"

/* LCD size */
#define TFT_H_RESOLUTION   (240)
#define TFT_V_RESOLUTION   (240)

/* LCD settings */
#define SPI_NUM                  (SPI3_HOST)
#define TFT_PIXEL_CLK_HZ         (40 * 1000 * 1000)
#define TFT_CMD_BITS             (8)
#define TFT_PARAM_BITS           (8)
#define TFT_COLOR_SPACE          (ESP_LCD_COLOR_SPACE_BGR)
#define TFT_BITS_PER_PIXEL       (16)
#define TFT_DRAW_BUFFER_DOUBLE   (1)
#define TFT_DRAW_BUFFER_HEIGHT   (50)

#define TAG  "[LCD]"

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

static lv_disp_t *lvgl_disp = NULL;

lv_obj_t *background;
lv_obj_t *background_title;
lv_obj_t *label_title;
lv_obj_t *lable_wifi;
lv_obj_t *lable_distance;
lv_obj_t *lable_capture;


static esp_err_t lcd_init(void)
{
   esp_err_t ret = ESP_OK;

   gpio_config_t bk_gpio_config = {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << GPIO_BL
   };

   ret = gpio_config(&bk_gpio_config);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "Failed to configure backlight");
   }

   const spi_bus_config_t buscfg = {
      .sclk_io_num = TFT_CLK,
      .mosi_io_num = TFT_MOSI,
      .miso_io_num = GPIO_NUM_NC,
      .quadwp_io_num = GPIO_NUM_NC,
      .quadhd_io_num = GPIO_NUM_NC,
      .max_transfer_sz = TFT_H_RESOLUTION * TFT_DRAW_BUFFER_HEIGHT * sizeof(uint16_t),
   };

   ret = spi_bus_initialize(SPI_NUM, &buscfg, SPI_DMA_CH_AUTO);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "Failed to init SPI");
      return ret;
   }

   const esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = TFT_DC,
      .cs_gpio_num = TFT_CS,
      .pclk_hz = TFT_PIXEL_CLK_HZ,
      .lcd_cmd_bits = TFT_CMD_BITS,
      .lcd_param_bits = TFT_PARAM_BITS,
      .spi_mode = 0,
      .trans_queue_depth = 10,
   };

   ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI_NUM, &io_config, &lcd_io);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "Failed New Panel IO");
      spi_bus_free(SPI_NUM);
   }

   const esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = TFT_RST,
      .color_space = TFT_COLOR_SPACE,
      .bits_per_pixel = TFT_BITS_PER_PIXEL,
   };

   ret = esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel);
   if (ret != ESP_OK)
   {
      esp_lcd_panel_io_del(lcd_io);
      spi_bus_free(SPI_NUM);
      ESP_LOGE(TAG, "Failed New Panel ST7789");
   }

   esp_lcd_panel_reset(lcd_panel);
   esp_lcd_panel_init(lcd_panel);
   esp_lcd_panel_disp_on_off(lcd_panel, true);

   gpio_set_level(GPIO_BL, 1);

   return ret;
}

static esp_err_t lvgl_init(void)
{
   esp_err_t ret;

   const lvgl_port_cfg_t lvgl_cfg = {
      .task_priority = 4,         /* LVGL task priority */
      .task_stack = 4096,         /* LVGL task stack size */
      .task_affinity = -1,        /* LVGL task pinned to core (-1 is no affinity) */
      .task_max_sleep_ms = 500,   /* Maximum sleep in LVGL task */
      .timer_period_ms = 5        /* LVGL timer tick period in ms */
   };

   ret = lvgl_port_init(&lvgl_cfg);
   if (ret != ESP_OK)
   {
      ESP_LOGE(TAG, "LVGL port initialization failed");
      return ret;
   }
   
   const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = lcd_io,
      .panel_handle = lcd_panel,
      .buffer_size = TFT_H_RESOLUTION * TFT_DRAW_BUFFER_HEIGHT* sizeof(uint16_t),
      .double_buffer = TFT_DRAW_BUFFER_DOUBLE,
      .hres = TFT_H_RESOLUTION,
      .vres = TFT_V_RESOLUTION,
      .monochrome = false,
      .rotation = {
         .swap_xy = false,
         .mirror_x = true,
         .mirror_y = true,
      },
      .flags = {
         .buff_dma = true,
      }
   };

   lvgl_disp = lvgl_port_add_disp(&disp_cfg);
   if (lvgl_disp == NULL)
   {
      ret = ESP_FAIL;
   }

   return ret;
}

static void main_screen(void)
{
   lv_obj_t *scr = lv_scr_act();

   lvgl_port_lock(0);
   background = lv_obj_create(scr);
   lv_obj_set_size(background,TFT_H_RESOLUTION,TFT_V_RESOLUTION);
   lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
   // lv_style_set_bg_color(background,lv_color_white());
   
   // background_title = lv_obj_create(scr);
   // lv_obj_set_size(background_title,240,30);
   // lv_obj_align(background_title, LV_ALIGN_TOP_MID, 0, 0);
   // lv_style_set_bg_color(background_title,lv_palette_main(LV_PALETTE_BLUE));


   lvgl_port_unlock();
}

void init_screen(void)
{

   ESP_ERROR_CHECK(lcd_init());
   ESP_ERROR_CHECK(lvgl_init());

   main_screen();
}