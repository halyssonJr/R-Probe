#include "common.h"
#include "vl53l0xV2.h"
#include "camera.h"
#include "display.h"
#include "wifi.h"
#include "web_server.h"

void app_main(void)
{
   // init_sensor();
   
   camera_init();
  
   //init_screen();
   wifi_init();

   web_server_setup();
}
