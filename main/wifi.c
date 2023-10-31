#include "common.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_mac.h"

#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#define SSID "TESLA"//"Roadhouse Blues - 2.4G"
#define PASSWORD "passwd39"//"senhaittnet2021"

#define TAG "[WIFI]"
#define RETRY_NUM 5

int wifi_connect_status = 0;
int retry_num = 0;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

   if (event_id == WIFI_EVENT_STA_START)
   {
      ESP_LOGI(TAG, "Connecting ...");
      esp_wifi_connect();
   }

   else if (event_id == WIFI_EVENT_STA_CONNECTED)
   {
      ESP_LOGI(TAG, "Connected");
      wifi_connect_status = 1;
   }

   else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
   {
      ESP_LOGI(TAG, "Desconnected");
      wifi_connect_status = 0;

      if (retry_num < RETRY_NUM)
      {
         esp_wifi_connect();
         retry_num++;
         ESP_LOGI(TAG, "Reconnecting ...");
      }
   }

   else if (event_id == IP_EVENT_STA_GOT_IP)
   {
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

      ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));
      retry_num = 0;
   }
   // Events related with Acess Point
   else if (event_id == WIFI_EVENT_AP_STACONNECTED)
   {
       wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
   }
   else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
   {
       wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
   }
}

void wifi_ap_init(void)
{
   esp_netif_init();
   esp_event_loop_create_default();
   esp_netif_t * esp_netif_handler = esp_netif_create_default_wifi_ap();
   if (esp_netif_handler == NULL)
   {
      ESP_LOGE(TAG, "Handler failed");
      return;
   }

   esp_netif_set_default_netif(esp_netif_handler);

   esp_netif_ip_info_t ip_info;
   esp_netif_set_ip4_addr(&ip_info.ip, 2, 2, 2, 1);
   esp_netif_set_ip4_addr(&ip_info.gw, 2, 2, 2, 1);
   esp_netif_set_ip4_addr(&ip_info.netmask, 255, 255, 255, 0);

   
   esp_netif_dhcps_stop(esp_netif_handler);
   esp_netif_set_ip_info(esp_netif_handler, &ip_info);
   esp_netif_dhcps_start(esp_netif_handler);

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

   esp_event_handler_instance_t instance_any_id;
   esp_event_handler_instance_t instance_got_ip;

   ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_any_id));
   ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip));
   wifi_config_t wifi_cfg =
   {
      .ap =
      {
         .max_connection = 1,
         .ssid = "R-Probe",
         .password = "",
         .authmode = WIFI_AUTH_OPEN,
      },
   };

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg));
   ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_sta_init()
{
   ESP_ERROR_CHECK(esp_netif_init());

   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_netif_create_default_wifi_sta();

   wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
   ESP_ERROR_CHECK(esp_wifi_init(&cfg));

   esp_event_handler_instance_t instance_any_id;
   esp_event_handler_instance_t instance_got_ip;
   ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_any_id));
   ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                      IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      &instance_got_ip));
   wifi_config_t wifi_config = 
   {
      .sta = 
      {
         .ssid = SSID,
         .password = PASSWORD,
      },
   };

   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
   ESP_ERROR_CHECK(esp_wifi_start());

   ESP_LOGI(TAG, "SSID:%s  password:%s", SSID, PASSWORD);
}

void wifi_init(void)
{
   esp_err_t ret = nvs_flash_init();
   if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
   {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
   }
   ESP_ERROR_CHECK(ret);
   wifi_ap_init();
}
