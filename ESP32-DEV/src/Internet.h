#pragma once

#include <WiFi.h>

#define WIFI_SSID "Nazih"
#define WIFI_PASSWORD "Nassar2002"

class Internet{
    public:
    Internet(){
        init_wifi();
    }

    void init_wifi()
    {

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    }

};


