idf_component_register(SRCS "app_main.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif
                    INCLUDE_DIRS "."
                    REQUIRES mqtt wifi_provisioning nvs_flash driver)