/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_tls.h"
#include "dumpcode.h"
#include "printer.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
static const char *TAG = "HTTP_CLIENT";

/* Root cert for howsmyssl.com, taken from howsmyssl_com_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const char letsencryptauthorityx3_pem_start[] asm("_binary_letsencryptauthorityx3_pem_start");
extern const char letsencryptauthorityx3_pem_end[]   asm("_binary_letsencryptauthorityx3_pem_end");

// CalDAV ZALGO parser
int state = 0;
int evtcnt = 0;

void caldav_init() {
    state = 0;
    evtcnt = 0;
}

char caldav_event_name[128];
char caldav_event_dtstart[128];
char caldav_event_duration[128];

// Given event msg is always trustable.
// because it is sanitized
void caldav_evt(char* str) {
    switch(state) {
        case 0:
        if(strcmp(str, "BEGIN:VEVENT") == 0) {
            state = 1;
        }
        break;
        case 1:
	if(strcmp(str, "END:VEVENT") == 0) {
	    char buf[128];
	    snprintf(buf, 128, "오늘의 %d 번째 일정", evtcnt + 1);
	    println(buf);
	    snprintf(buf, 128, "제목: %s", caldav_event_name);
	    println(buf);
	    snprintf(buf, 128, "시작 시각: %s", caldav_event_dtstart);
	    println(buf);
	    snprintf(buf, 128, "소요 시간: %s", caldav_event_duration);
	    println(buf);
            state = 0;
	    evtcnt++;
	    break;
	}

	if(strstr(str, "SUMMARY") != NULL) {
	    strncpy(caldav_event_name, str + 8, 128);
	} else if (strstr(str, "DTSTART") != NULL) {
            strncpy(caldav_event_dtstart, str + 8, 128);
	} else if (strstr(str, "DURATION") != NULL) {
	    strncpy(caldav_event_duration, str + 9, 128);
	} else if (strstr(str, "STATUS:CANCELLED") != NULL) {
	    state = 0;
	}
	break;
    }
}

char evtbuf[1024];
char *evtbufptr = evtbuf;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
	    caldav_init();
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
	    for(int i = 0; i < evt -> data_len; i++) {
		char data = ((char*)evt -> data)[i];
		if (data == 0x0D || data == 0x0A) {
		    *evtbufptr = 0x00;
		    printf("%s\n", evtbuf);
		    caldav_evt(evtbuf);
		    evtbufptr = evtbuf;
		} else {
		    *evtbufptr = data;
		    evtbufptr ++;
		}
	    }
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Write out data
                //printf("%.*s", evt->data_len, (char*)evt->data);
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
	    print_feed(10);
	    println("좋은 하루 되세요!");
	    print_cut();
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
    }
    return ESP_OK;
}

static void https_with_url(void)
{
    char *buffer = malloc(MAX_HTTP_RECV_BUFFER + 1);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Cannot malloc http receive buffer");
        return;
    }
    esp_http_client_config_t config = {
        .url = CONFIG_CALDAV_URL,
	.username = CONFIG_CALDAV_USERNAME,
	.password = CONFIG_CALDAV_PASSWORD,
        .event_handler = _http_event_handler,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .cert_pem = letsencryptauthorityx3_pem_start,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    char post_data[512];
    //snprintf(post_data, 512, "<c:calendar-query xmlns:c=\"urn:ietf:params:xml:ns:caldav\"><d:prop xmlns:d=\"DAV:\"><d:getetag/><c:calendar-data/></d:prop><c:filter><c:comp-filter name=\"VCALENDAR\"><c:comp-filter name=\"VEVENT\"><c:time-range start=\"%s\" end=\"%s\"/></c:comp-filter></c:comp-filter></c:filter></c:calendar-query>", "20181125T150000Z", "20190106T150000Z");
    snprintf(post_data, 512, "<c:calendar-query xmlns:c=\"urn:ietf:params:xml:ns:caldav\"><d:prop xmlns:d=\"DAV:\"><d:getetag/><c:calendar-data/></d:prop><c:filter><c:comp-filter name=\"VCALENDAR\"><c:comp-filter name=\"VEVENT\"><c:time-range start=\"%s\" end=\"%s\"/></c:comp-filter></c:comp-filter></c:filter></c:calendar-query>", "20191220T150000Z", "20191226T150000Z");
    esp_http_client_set_method(client, HTTP_METHOD_REPORT);
    esp_http_client_set_header(client, "Depth", "1");
    esp_http_client_set_header(client, "Content-Type", "application/xml; charset=utf-8");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

static void http_test_task(void *pvParameters)
{
    https_with_url();

    ESP_LOGI(TAG, "Finish http example");
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    printer_init();

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP, begin http example");

    xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);
}
