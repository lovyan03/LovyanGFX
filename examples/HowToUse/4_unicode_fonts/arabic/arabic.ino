#include "main/main.cpp"

void setup() {
    xTaskCreate(app, "app", 8192, NULL, 1, NULL);
}

void loop() {}
