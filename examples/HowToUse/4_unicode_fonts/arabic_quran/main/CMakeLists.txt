idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES "LovyanGFX"
    PRIV_REQUIRES "esp_wifi" "esp_http_client" "esp-tls" "spiffs" "mbedtls"
)

