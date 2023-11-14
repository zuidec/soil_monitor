/*
 *  Arduino pro mini 3.3 app to read a capacitive moisture sensor
 *  then transmit the reading using NRF24L01 to a base station
 * 
 *  Author: Case Zuiderveld
 *  Last updated 5/28/2023
 */

#include <Arduino.h>
// SPI and RF24 are needed to interface with NRF24L01 radio module 
#include <SPI.h>
#include <RF24.h>
// LowPower library for sleeping functionality
#include <LowPower.h>
// SoilMonitor.h has the functions to read soil sensor and use autowater
#include "SoilMonitor.h"
// PlantPacket.h has the functions to make packets for sending wirelessly
#include "PlantPacket.h"

// Define all the pins so we can call them by name
#define SOIL_SENSOR_PWR_PIN   (5)
#define SOIL_SENSOR_DATA_PIN  (A0)
#define NRF24L01_MOSI_PIN     (11)
#define NRF24L01_MISO_PIN     (12)
#define NRF24L01_SCK_PIN      (13)
#define NRF24L01_CSN_PIN      (10)
#define NRF24L01_CE_PIN       (9)
#define PUMP_PWR_PIN          (3)
#define FLOAT_SENSOR_PIN      (4)

#define TIME_TO_SLEEP_SECONDS 14400   // 3,600s in one hour
#define BUFFER_LENGTH         16

// Objects
RF24 radio(NRF24L01_CE_PIN, NRF24L01_CSN_PIN);
SoilMonitor soilMonitor(SOIL_SENSOR_PWR_PIN,SOIL_SENSOR_DATA_PIN, PUMP_PWR_PIN, FLOAT_SENSOR_PIN);
PlantPacket packet;

// Variables and constants
uint8_t radioAddress[6] = {"ollie"};
uint8_t baseStationAddress[5] = {'b','a','s','e','\0'};
char plantName[15] = {'o','l','i','v','e','r','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
char buffer[BUFFER_LENGTH]    = {"\0"};

// Functions
bool InitializeRadio();
void ClearBuffer(char *buffer, int bufferLength);
void EnterSleepMode(uint16_t timeToSleepSeconds);

//
// 
//
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    soilMonitor.autoWater = false;

    if(!InitializeRadio())  {
      Serial.println(F("Failed to initialize radio"));
    }
    
    packet.SetPlantPacketName(&plantName[0]);
}

void loop() {
    // Read soil level
    soilMonitor.ReadSoilLevel();
    packet.percentSoilLevel = soilMonitor.percentSoilLevel;
    ClearBuffer(&buffer[0], BUFFER_LENGTH);
    packet.CreatePlantPacket(&buffer[0]);
    
    // Ouput buffer contents for packet debugging
    Serial.print("Buffer contents: ");
    for(int i=0;i<16;i++)  {
      Serial.print(buffer[i]);
    }
    Serial.println();
    
    // Attempt to transmit the soil level
    if(!radio.write(&buffer[0], sizeof(buffer)))  {
      Serial.println(F("Transmission failed"));
    }
    else  {
      Serial.println(F("Transmission successful"));
    }
    
    // Go to sleep
    EnterSleepMode(TIME_TO_SLEEP_SECONDS);
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
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_2MBPS);
    radio.setPayloadSize(sizeof(buffer));
    radio.openWritingPipe(baseStationAddress);
    radio.stopListening();

    return true;
}

void ClearBuffer(char *buffer, int bufferLength) {

  for(int i=0; i<bufferLength; i++) {
    buffer[i] = '\0';
  }

  return;
}

void EnterSleepMode(uint16_t timeToSleepSeconds) {

  // Put radio into powerdown mode
  radio.powerDown();
  
  // Loop through sleeping 8s at a time until its time to wake up
  for (uint16_t i = 0; (timeToSleepSeconds-i) >= 8; i+=8)
  {
    // Put arduino into power down mode
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  
  // Turn radio back on
  radio.powerUp();
  
}