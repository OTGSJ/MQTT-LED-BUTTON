#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define WIFI_SSID      "GalaxyA10"
#define WIFI_PASS      "jose1234"
#define BROKER_URI     "mqtt://test.mosquitto.org"

#define LED_GPIO       GPIO_NUM_13
#define BUTTON_GPIO    GPIO_NUM_14

#define TOPIC_CMD      "home/led/command"
#define TOPIC_STATUS   "home/led/status"

static const char *TAG = "MQTT_LED";
static esp_mqtt_client_handle_t client;
static bool led_on = false;

static void set_led(bool on)
{
    led_on = on;
    gpio_set_level(LED_GPIO, on);
    ESP_LOGI(TAG, "LED %s", on ? "ON" : "OFF");
    esp_mqtt_client_publish(client, TOPIC_STATUS, on ? "ON" : "OFF", 0, 1, 1);
}

static void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    esp_mqtt_event_handle_t event = data;

    switch (id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");        
            esp_mqtt_client_subscribe(client, TOPIC_CMD, 1);  
            set_led(led_on);                             
            break;

        case MQTT_EVENT_DATA: {
            if (strncmp(event->topic, TOPIC_CMD, event->topic_len) != 0) break;
            char payload[8] = {0};
            memcpy(payload, event->data, event->data_len > 7 ? 7 : event->data_len);
            set_led(strncmp(payload, "ON", 2) == 0);
            break;
        }
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT desconectado");
            break;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t base, int32_t id, void* data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi conectado, IP recebido");
        esp_mqtt_client_start(client);
    }
}

static void button_task(void *pvParameters)
{
    bool last = true; 
    
    while (1) {
        bool cur = gpio_get_level(BUTTON_GPIO);
        if (last != cur) { 
            vTaskDelay(pdMS_TO_TICKS(50));
            if (last != gpio_get_level(BUTTON_GPIO)) { 
                const char *command = !gpio_get_level(BUTTON_GPIO) ? "ON" : "OFF";
                esp_mqtt_client_publish(client, TOPIC_CMD, command, 0, 1, 0);
            }
        }
        last = cur;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) 
{
    // GPIO
    gpio_reset_pin(LED_GPIO);    
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BUTTON_GPIO); 
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_GPIO);

    // NVS + rede
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_cfg = {
        .sta = { .ssid = WIFI_SSID, .password = WIFI_PASS },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    // MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URI,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Task do botão
    xTaskCreate(button_task, "btn", 2048, NULL, 5, NULL);
}