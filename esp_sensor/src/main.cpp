/*
 *	main.cpp
 *	Soil monitor for ESP32
 *
 *	Created by zuidec on 11/13/23
 */

#include <Arduino.h>
// cstdio for sprintf
#include <cstdio>
#include <sys/_stdint.h>
// WiFi and HTTPClient for transmitting soil levels to the database
#include <WiFi.h>
#include <HTTPClient.h>
// wifiFix needed to fix a bug with the WiFi library
#include "wifiFix.h"
#include "esp_sleep.h"
// serverName, ssid, password, ntfyServer, and apiKey are all defined in credentials.h
#include "credentials.h"

// These define macros are needed to pass the plantName to the compiler as a 
// define at build time
#define STR(s)                  ST(s)
#define ST(s)                   #s

#define SOIL_RX_PIN             (3) 
#define SOIL_PWR_PIN            (18) 
#define SLEEP_TIME_MS           (14400000000)
#define WIFI_TIMEOUT_MS         (10000)
#define MS_PER_S                (1000)

WiFiClient* client = new WiFiClientFixed();

const char* plantName           = STR(PLANT_NAME); 
const int air_moisture          = 1024;
const int water_moisture        = 500;

int ReadSoilLevel();
bool InitializeWifi();
bool IsWiFiReady();  
void UpdateMoistureDatabase(const char* plantName, int percentMoisture);
void SendPushNotification(const char* notification, const char* topic);
void UpdatePushNotifications(const char* plantName, int percentMoisture);

void setup() {
    
    Serial.begin(115200);
    analogReadResolution(10);
    pinMode(SOIL_PWR_PIN, OUTPUT);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_MS);

    if(!InitializeWifi()) {
        Serial.println("Failed to initialize WiFi");
        delay(WIFI_TIMEOUT_MS); 
        Serial.println("Returning to sleep"); 
        esp_deep_sleep_start();
    }

}

void loop() {

    int soilLevel = ReadSoilLevel();
    UpdateMoistureDatabase(plantName, soilLevel);
    UpdatePushNotifications(plantName, soilLevel);

    Serial.println("Going to sleep...");
    esp_deep_sleep_start();
}

bool InitializeWifi() {

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");

    for (int i=0; WiFi.status() != WL_CONNECTED && i<=(WIFI_TIMEOUT_MS/MS_PER_S); i++) {
        Serial.print('.');
        delay(1000);

        if(i==(WIFI_TIMEOUT_MS/MS_PER_S)) {
            Serial.println(" connection timed out!");
            WiFi.disconnect(true, true);
            return false;
        }
    }

    Serial.print(" connected! IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool IsWiFiReady()  {

    if(WiFi.status() != WL_CONNECTED) {
        Serial.print("Problem with wifi connection, attempting to reconnect... ");
        WiFi.disconnect();
        WiFi.begin(ssid, password);

        if(WiFi.waitForConnectResult(WIFI_TIMEOUT_MS)!=WL_CONNECTED)  {
            WiFi.disconnect(true,true);
            Serial.println("reconnect failed!");
            return false;
        }

        Serial.print("reconnect successful! IP address: ");
        Serial.println(WiFi.localIP());
    }

    return true;
}

int ReadSoilLevel()    {

    digitalWrite(SOIL_PWR_PIN, HIGH);
    delay(250);
    int rawSoilMoisture = analogRead(SOIL_RX_PIN);
    digitalWrite(SOIL_PWR_PIN, LOW);

    return map(rawSoilMoisture, air_moisture, water_moisture, 0, 100);
}

void UpdateMoistureDatabase(const char* plant, int percentMoisture) {
  
    if(!IsWiFiReady())  {
        Serial.println("Database updated aborted, wifi is not connected!");
        return;
    }

    HTTPClient http;
    http.begin(*client, serverName);
    http.addHeader("Content-Type","application/x-www-form-urlencoded");

    String httpRequestData = "api_key=" +  apiKeyValue + "&moisture=" + percentMoisture + "%&plantname=" + plant + "";
    Serial.print("Database request data: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode==200) {
        Serial.println("Database updated successfully!");
        http.end();
    }
    else if (httpResponseCode>0) {
        Serial.print("Unknown database update result, http response code: ");
        Serial.println(httpResponseCode);
        http.end();
    }
    else {
        Serial.print("Database update failed, http response code: ");
        Serial.println(httpResponseCode);
        http.end();
    }
}

void SendPushNotification(const char* notification, const char* topic)    {

    if(!IsWiFiReady())  {
        Serial.println("Push notification aborted, wifi is not connected!");
        return;
    }

    HTTPClient http;
    char address[64] = "";
    sprintf(address, "%s:8080/%s", ntfyServer, topic);  
    http.begin(*client, address);
    http.addHeader("Content-Type","text/plain");

    Serial.print("Push notification request data: ");
    Serial.println(notification);

    int httpResponseCode = http.POST(notification);


    if (httpResponseCode==200) {
        Serial.println("Push notification sent successfully!");
        http.end();
    }
    else if (httpResponseCode>0) {
        Serial.print("Unknown notification result, http response code: ");
        Serial.println(httpResponseCode);
        http.end();
    }
    else {
        Serial.print("Push notifcation failed, http response code: ");
        Serial.println(httpResponseCode);
        http.end();
    }
} 

void UpdatePushNotifications(const char* plantName, int percentMoisture)   {

    if(percentMoisture <= 50)   {
        SendPushNotification("Soil moisture is below 50%, water soon!", plantName);
    }
    else if(percentMoisture <= 40)   {
        SendPushNotification("Soil moisture is below 40%, water now!", plantName);
    }
}
