#if 0
#include <esp_err.h>
#include <esp_log.h>

#include <driver/adc.h>
#include <driver/timer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lwip/api.h>

#include <string.h>

#include "car.h"
#include "proto.h"
#include "sdkconfig.h"

#define MIC_ADC_CHANNEL ADC1_CHANNEL_5

static const char *TAG = "mic";

static volatile int16_t samples[3200] = {0};
static volatile uint8_t sample_page = 0;
static volatile bool ready = false;

static void mic_task(void *params) {
   struct netconn *conn = netconn_new(NETCONN_UDP);
   assert(conn != NULL);

   while(1) {
      if(!ready) continue;
      ip_addr_t dest_addr;
      ipaddr_aton("192.168.0.30", &dest_addr);

      for(int i = 0; i < 1600 * 2; i += 400) {
         struct netbuf buf = {0};
         uint8_t *buf_data = netbuf_alloc(&buf, 400);
         if(!buf_data) break;

         memcpy(buf_data, (uint8_t*) samples + (sample_page ^ 1) * 1600 * 2 + i, 400);

         err_t err = netconn_sendto(conn, &buf, &dest_addr, CONFIG_CAMERA_PORT);
         netbuf_free(&buf);
         if(err != ERR_OK) break;
      }

      ready = false;

      vTaskDelay(50 / portTICK_PERIOD_MS);
   }
}

static bool IRAM_ATTR mic_isr(void *params) {
   static int i = 0;
   int val = adc1_get_raw(MIC_ADC_CHANNEL);
   samples[sample_page * 1600 + (i++)] = val < 0x10000 ? val : 0xFFFF;
   if(i >= 1600) {
      if(!ready) {
         sample_page ^= 1;
         ready = 1;
      }
      i = 0;
   }
   return false;
}

void mic_init(void) {
   ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
   ESP_ERROR_CHECK(adc1_config_channel_atten(MIC_ADC_CHANNEL, ADC_ATTEN_DB_11));

    xTaskCreate(mic_task, TAG, 8192, NULL, tskIDLE_PRIORITY+1, NULL);

   timer_config_t timer_config = {
      .alarm_en     = TIMER_ALARM_EN,
      .counter_en   = TIMER_PAUSE,
      .counter_dir  = TIMER_COUNT_UP,
      .auto_reload  = true,
      .divider      = 50,
   };

   ESP_ERROR_CHECK(timer_init(TIMER_GROUP_0, TIMER_0, &timer_config));
   ESP_ERROR_CHECK(timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0));
   ESP_ERROR_CHECK(timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 100));
   ESP_ERROR_CHECK(timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, mic_isr, NULL, 0/*ESP_INTR_FLAG_IRAM*/));
   ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0));
}
#endif
