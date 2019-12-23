# Morning print

## Note
This project requires patched esp-idf to build because `esp_http` does not support REPORT method yet.

```
diff --git a/components/esp_http_client/esp_http_client.c b/components/esp_http_client/esp_http_client.c
index 3c6b08c64..ab7888ad9 100644
--- a/components/esp_http_client/esp_http_client.c
+++ b/components/esp_http_client/esp_http_client.c
@@ -150,7 +150,8 @@ static const char *HTTP_METHOD_MAPPING[] = {
     "NOTIFY",
     "SUBSCRIBE",
     "UNSUBSCRIBE",
-    "OPTIONS"
+    "OPTIONS",
+    "REPORT"
 };
 
 static esp_err_t esp_http_client_request_send(esp_http_client_handle_t client, int write_len);
diff --git a/components/esp_http_client/include/esp_http_client.h b/components/esp_http_client/include/esp_http_client.h
index 8d9243a0b..7dd533528 100644
--- a/components/esp_http_client/include/esp_http_client.h
+++ b/components/esp_http_client/include/esp_http_client.h
@@ -83,6 +83,7 @@ typedef enum {
     HTTP_METHOD_SUBSCRIBE,  /*!< HTTP SUBSCRIBE Method */
     HTTP_METHOD_UNSUBSCRIBE,/*!< HTTP UNSUBSCRIBE Method */
     HTTP_METHOD_OPTIONS,    /*!< HTTP OPTIONS Method */
+    HTTP_METHOD_REPORT,     /*!< HTTP REPORT Method */
     HTTP_METHOD_MAX,
 } esp_http_client_method_t;
```
