#include "vl53l0x.h"
#include "driver/i2c.h"

#include "common.h"

#define I2C_PIN_SDA 21
#define I2C_PIN_SCL 22
#define I2C_PORT 1
#define XSHUT -1
#define VL53L0X_ADDR 0x29
#define TAG "[VL53L0X]"

#define VL53L0X_TASK_STACK_SIZE 4096
#define VL53L0X_TASK_PRIORITY   3
#define VL53L0X_MESSAGE_BUFFER_SIZE 100
vl53l0x_t * vl53l0x_cfg;
TaskHandle_t vl53l0x_handler;
MessageBufferHandle_t xmsg_buffer;

static void vl53l0x_task(void *pvParameters)
{
	uint16_t distance;

	while (true)
	{
		distance = vl53l0x_readRangeContinuousMillimeters(vl53l0x_cfg);
      ESP_LOGI(TAG, "Distance =  %d mm", distance);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelete(NULL);
}

esp_err_t vl53l0x_setup(void)
{
	esp_err_t ret  = ESP_FAIL; 

	vl53l0x_cfg = (vl53l0x_t *)malloc (sizeof(vl53l0x_t *));
	vl53l0x_cfg = vl53l0x_config(I2C_PORT, I2C_PIN_SCL, I2C_PIN_SDA, XSHUT, VL53L0X_ADDR, -1 );

	if (vl53l0x_cfg == NULL || vl53l0x_cfg == 0)
	{
		ESP_LOGE(TAG, "Failed to configurte vl53l0x");
		return ret;
	}

	if (vl53l0x_init(vl53l0x_cfg) != NULL)
	{
		ESP_LOGI(TAG, "Failed to init vl53l0x");
		return ret;
	}

	if (vl53l0x_setVcselPulsePeriod(vl53l0x_cfg,VcselPeriodPreRange, 12) != NULL)
	{
		ESP_LOGI(TAG, "Failed to set VCSEL Pre Range");
		return ret;
	}
	if (vl53l0x_setVcselPulsePeriod(vl53l0x_cfg,VcselPeriodFinalRange, 8) != NULL)
	{
		ESP_LOGI(TAG, "Failed to set VCSEL Final Range");
		return ret;
	}

	vl53l0x_startContinuous(vl53l0x_cfg, 0);
	ret = ESP_OK;
	ESP_LOGI(TAG, "vl53l0x Init");

	return ret;
}

esp_err_t init_sensor(void)
{
	esp_err_t ret = ESP_FAIL;

	ret = vl53l0x_setup();
	if (ret != ESP_OK)
	{
		return ret;
	}

	if (xTaskCreate(vl53l0x_task, "VL53L0X Task", VL53L0X_TASK_STACK_SIZE,
						NULL, VL53L0X_TASK_PRIORITY, &vl53l0x_handler) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create task");
		return ret;
	}

	xmsg_buffer = xMessageBufferCreate(VL53L0X_MESSAGE_BUFFER_SIZE);
	if (xmsg_buffer != NULL)
	{
		ret = ESP_OK;
	}
	else
	{	
		ESP_LOGE(TAG, "Failed to create Message Buffer");
		vTaskDelete(vl53l0x_handler);
	}

	return ret;
}