/*
 *  ESP32 app to receive soil moisture data
 *  then transmit to an online database
 * 
 *  Author: Case Zuiderveld
 *  Last updated 5/28/2023
 */

#include <Arduino.h>
// RF24 and SPI for radio communication
#include <RF24.h>
#include <SPI.h>
// WiFi and HTTPClient for transmitting soil levels to the database
#include <WiFi.h>
#include <HTTPClient.h>
#include "wifiFix.h"    // Needed to fix a bug with the WiFi library
// FastLED for onboard RGB LED control
#include <FastLED.h>
// PlantPacket for using ParsePlantPacket()
#include "PlantPacket.h"
#include "credentials.h"

// Define physical pin layout
#define NRF24L01_MOSI_PIN     (6)
#define NRF24L01_MISO_PIN     (5)
#define NRF24L01_SCK_PIN      (4)
#define NRF24L01_CSN_PIN      (10)
#define NRF24L01_CE_PIN       (7)
#define LED_PIN               (8)

#define BUFFER_LENGTH         16
#define NUM_LEDS              1

// Objects
RF24 radio(NRF24L01_CE_PIN, NRF24L01_CSN_PIN);
PlantPacket packet;
WiFiClient* client = new WiFiClientFixed();
CRGB led[NUM_LEDS] = {0};

// Variables
uint8_t baseStationAddress[5] = {'b','a','s','e','\0'};
const char* plantName[6]      = {"oliver", "lily", "gustav", "thumbelina", "ivy", "champ"};
uint8_t buffer[BUFFER_LENGTH]    = {"\0"};

// Functions
bool InitializeRadio();
bool InitializeWifi();
bool UpdateMoistureDatabase(const char* plantName, int percentMoisture);
void ClearBuffer(uint8_t *buffer, int bufferLength);
void SetLEDColor(CRGB color);

//
//
//
void setup() {

  Serial.begin(115200);
  if(!InitializeRadio())  {
    Serial.println("Failed to initialize radio");
  }

  if(!InitializeWifi()) {
    Serial.println("Failed to initialize WiFi");
  }
  Serial.print("ssid: ");
  Serial.print(ssid);
  Serial.print("  password: ");
  Serial.print(password);
  Serial.print("  server name: ");
  Serial.print(serverName);
  Serial.print("  api key: ");
  Serial.println(apiKeyValue);
  // Initialize onboard status LED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(led, NUM_LEDS);
  FastLED.setBrightness(24);
  // Set LED once setup is complete
  SetLEDColor(CRGB::Green);
  Serial.println("Waiting for plant packets....");
}

void loop() {
  
  if(radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    packet.ParsePlantPacket(&buffer[0]);
    ClearBuffer(&buffer[0], BUFFER_LENGTH);
    Serial.print(packet.plantName);
    Serial.println(packet.percentSoilLevel); 

   UpdateMoistureDatabase(packet.plantName, (int)packet.percentSoilLevel);
  }

}

bool InitializeRadio()  {

  // Attempt to connect to radio up to three times
  for(uint8_t i=0; i<3 && !radio.begin(); i++)  {
    if(i==3)  {
      return false;
    } 
    delay(50); // Wait 50ms before attempting radio initialization again
  }

  // Initialize radio settings
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);
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

  // Set Wifi connection settings and attempt to connect to network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  for (int i=0; WiFi.status() != WL_CONNECTED; i<=10) {
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
  
  // Check if WiFi is connected and attempt one reconnect if it isn't
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
    if(WiFi.status() != WL_CONNECTED) {
      SetLEDColor(CRGB::Red);
      return false;
    }
  }
    
  HTTPClient http;

  // Create http client to connect to server
  http.begin(*client, serverName);
  http.addHeader("Content-Type","application/x-www-form-urlencoded");

  // Set up a string with our post request
  String httpRequestData = "api_key=" +  apiKeyValue + "&moisture=" + percentMoisture + "%&plantname=" + plant + "";
  Serial.print("httpRequestData: ");
  Serial.println(httpRequestData);

  // Send HTTP POST request
  int httpResponseCode = http.POST(httpRequestData);
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
    SetLEDColor(CRGB::Green);
    return true;
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    http.end();

    return false;
  }

}


void ClearBuffer(uint8_t *buffer, int bufferLength) {

  for(int i=0; i<bufferLength; i++) {
    buffer[i] = '\0';
  }

  return;
}

void SetLEDColor(CRGB color)  {
  FastLED.clear();
  led[0] = color;
  FastLED.show();
}