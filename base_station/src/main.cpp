/*
 *  ESP32 app to receive soil moisture data
 *  then transmit to an online database
 * 
 *  Author: Case Zuiderveld
 *  Last updated 1/17/2024
 */

#include <Arduino.h>
// cstdio for sprintf
#include <cstdio>
// RF24 and SPI for radio communication
#include <RF24.h>
#include <SPI.h>
// WiFi and HTTPClient for transmitting soil levels to the database
#include <WiFi.h>
#include <HTTPClient.h>
// wifiFix needed to fix a bug with the WiFi library
#include "wifiFix.h"    
// FastLED for onboard RGB LED control
#include <FastLED.h>
// PlantPacket for using ParsePlantPacket()
#include "PlantPacket.h"
// serverName, ssid, password, ntfyServer, and apiKey are all defined in credentials.h
#include "credentials.h"

#define NRF24L01_MOSI_PIN       (6)
#define NRF24L01_MISO_PIN       (5)
#define NRF24L01_SCK_PIN        (4)
#define NRF24L01_CSN_PIN        (10)
#define NRF24L01_CE_PIN         (7)
#define LED_PIN                 (8)

#define BUFFER_LENGTH           (16)
#define NUM_LEDS                (1)
#define TIMER_PERIOD_MS         (30000)

// Objects
RF24 radio(NRF24L01_CE_PIN, NRF24L01_CSN_PIN);
PlantPacket packet;
WiFiClient* client              = new WiFiClientFixed();
CRGB led[NUM_LEDS]              = {0};

// Variables
uint8_t baseStationAddress[5]   = {'b','a','s','e','\0'};
uint8_t buffer[BUFFER_LENGTH]   = {"\0"};
unsigned long timer             = 0;

// Functions
bool InitializeRadio();
bool InitializeWifi();
bool UpdateMoistureDatabase(const char* plantName, int percentMoisture);
bool SendPushNotification(const char* notification, const char* topic);
void UpdatePushNotifications(const char* plantName, int percentMoisture);
void ClearBuffer(uint8_t *buffer, int bufferLength);
void SetLEDColor(CRGB color);
void UpdateLED();

//
//
//
void setup() {

    Serial.begin(115200);
      
    // MUST delay here, the LED is on one of the strapping pins, attempting to
    // set too soon after reboot causes a boot loop
    delay(100);

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(led, NUM_LEDS);
    FastLED.setBrightness(10);
    SetLEDColor(CRGB::Blue);

    if(!InitializeRadio())  {
        Serial.println("Failed to initialize radio");
        SetLEDColor(CRGB::Red);
        delay(60000);
        setup();
    }

    if(!InitializeWifi()) {
        Serial.println("Failed to initialize WiFi");
        SetLEDColor(CRGB::Red);
        delay(60000);
        Serial.println("Returning to setup"); 
        setup();
    }

    //delay(1000);
    SetLEDColor(CRGB::Green);
    Serial.println("Waiting for plant packets...");
    timer = millis();
}

void loop() {
  
    if(radio.available()) {
        radio.read(&buffer, sizeof(buffer));
        packet.ParsePlantPacket(&buffer[0]);
        ClearBuffer(&buffer[0], BUFFER_LENGTH);
        Serial.print(packet.plantName);
        Serial.println(packet.percentSoilLevel); 

        UpdateMoistureDatabase(packet.plantName, (int)packet.percentSoilLevel);
        UpdatePushNotifications(packet.plantName, (int)packet.percentSoilLevel);

        Serial.println("Waiting for plant packets...");
    }

    if(timer - millis() >= TIMER_PERIOD_MS)   {
        UpdateLED();
        timer = millis();
    }
    if(timer > millis())    {
        timer = millis();
    }

}

bool InitializeRadio()  {

    for(uint8_t i=0; i<3 && !radio.begin(); i++)  {
        if(i==3)  {
            return false;
        } 
        delay(50); 
    }

    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(1, baseStationAddress);
    radio.setPayloadSize(sizeof(buffer));
    radio.flush_rx();
    radio.startListening();

    if(!radio.isChipConnected())  {
        Serial.println("Radio chip not connected");
        return false;
    }

    return true;
}

bool InitializeWifi() {

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");

    for (int i=0; WiFi.status() != WL_CONNECTED && i<=10; i++) {
        Serial.print('.');
        delay(1000);

        if(i==10) {
            return false;
        }
    }

    Serial.println(WiFi.localIP());
    return true;
}

bool UpdateMoistureDatabase(const char* plant, int percentMoisture) {
  
    if(WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();

        if(WiFi.status() != WL_CONNECTED) {
            SetLEDColor(CRGB::Red);
            return false;
        }
    }

    HTTPClient http;

    http.begin(*client, serverName);
    http.addHeader("Content-Type","application/x-www-form-urlencoded");

    String httpRequestData = "api_key=" +  apiKeyValue + "&moisture=" + percentMoisture + "%&plantname=" + plant + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        http.end();
        return true;
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
    
    if(percentMoisture <= 50)   {
        
    }
}

bool SendPushNotification(const char* notification, const char* topic)    {

    if(WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();

        if(WiFi.status() != WL_CONNECTED) {
            SetLEDColor(CRGB::Red);
            return false;
        }
    }

    HTTPClient http;
    char address[32] = "";
    sprintf(address, "http://%s:8080/%s", ntfyServer, topic);  
    
    http.begin(*client, address);
    http.addHeader("Content-Type","text/plain");

    Serial.print("Notification: ");
    Serial.println(notification);

    int httpResponseCode = http.POST(notification);

    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        http.end();
        return true;
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}    
void UpdatePushNotifications(const char* plantName, int percentMoisture)   {

    if(percentMoisture <= 50)   {
        bool status = SendPushNotification("Soil moisture is below 50%, water soon!", plantName);
        if(!status) {
            Serial.println("Notification failed");
        }
    }
    else if(percentMoisture <= 40)   {
        bool status = SendPushNotification("Soil moisture is below 40%, water now!", plantName);
        if(!status) {
            Serial.println("Notification failed");
        }
    }
}

void ClearBuffer(uint8_t *buffer, int bufferLength) {

    for(int i=0; i<bufferLength; i++) {
        buffer[i] = '\0';
    }
}

void SetLEDColor(CRGB color)  {

    FastLED.clear();
    led[0] = color;
    FastLED.show();
}

void UpdateLED()    {

    if(WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();

        if(WiFi.status() != WL_CONNECTED) {
            SetLEDColor(CRGB::Red);
        }
    }
    else    {
        SetLEDColor(CRGB::Green);
    }
}
