/*
 *  Arduino SoilMonitor library to read 
 *  moisture data and run the auto water feature
 * 
 *  Author: Case Zuiderveld
 *  Last updated: 5/8/2023
 */

#include "SoilMonitor.h"

SoilMonitor::SoilMonitor(uint8_t sensorPowerPin, uint8_t sensorDataPin, uint8_t pumpPowerPin, uint8_t floatSensorPin)  {
    
    // Set up the pinout
    SOILSENSOR_PWR_PIN  = sensorPowerPin;
    SOILSENSOR_DATA_PIN = sensorDataPin;
    PUMP_PWR_PIN        = pumpPowerPin;
    FLOAT_SENSOR_PIN    = floatSensorPin;
    // Configure digital pin modes. Soil data needs no configuration since it is an analog pin
    pinMode(SOILSENSOR_PWR_PIN, OUTPUT);
    pinMode(PUMP_PWR_PIN, OUTPUT);
    pinMode(FLOAT_SENSOR_PIN, INPUT_PULLUP);

    // Initialize calibration values to default for arduino pro mini
    CalibrateSensor(DEFAULT_MIN_MOISTURE, DEFAULT_MAX_MOISTURE);
    // Initialize pump thresholds to default values
    SetAutoWaterThresholds(DEFAULT_AUTOWATER_START_THRESHOLD, DEFAULT_AUTOWATER_SHUTOFF_THRESHOLD);
    // Enable auto water
    autoWater = true;
}

SoilMonitor::SoilMonitor(uint8_t sensorPowerPin, uint8_t sensorDataPin) {
    
    // Set up the pinout
    SOILSENSOR_PWR_PIN  = sensorPowerPin;
    SOILSENSOR_DATA_PIN = sensorDataPin;
    // Configure digital pin modes. Soil data needs no configuration since it is an analog pin
    pinMode(SOILSENSOR_PWR_PIN, OUTPUT);

    // Initialize calibration values to default for arduino pro mini
    CalibrateSensor(DEFAULT_MIN_MOISTURE, DEFAULT_MAX_MOISTURE);
    // Disable auto water
    autoWater = false;
}

SoilMonitor::~SoilMonitor() {

} 

void SoilMonitor::ReadSoilLevel()    {
    
    rawSoilLevel = 0;
    digitalWrite(SOILSENSOR_PWR_PIN, HIGH);
    delay(250); // Wait for the sensor to equalize

    // Take a few readings and add them together
    for(uint8_t i=0; i < SAMPLE_QUANTITY; i++)  {
        rawSoilLevel += analogRead(SOILSENSOR_DATA_PIN);
        delay(50);
    }
    digitalWrite(SOILSENSOR_PWR_PIN, LOW);
  
    // Then divide the readings by SAMPLE_QUANTITY to get the average 
    rawSoilLevel = rawSoilLevel/SAMPLE_QUANTITY;
    
    // Map to a percentage between 0% and 100%
    percentSoilLevel = map(rawSoilLevel, minMoistureLevel, maxMoistureLevel, 0, 100);
    
    // Jump to auto watering function if enabled and current moisture level is below the threshold
    if(autoWater && percentSoilLevel < autoWaterStartThreshold)   {
        BeginAutoWatering();
    }
}

void SoilMonitor::CalibrateSensor(uint16_t minLevel, uint16_t maxLevel) {
    
    // Calibrate sensor specific values to be used with the map function   
    minMoistureLevel = minLevel;
    maxMoistureLevel = maxLevel;
}

void SoilMonitor::SetAutoWaterThresholds(uint8_t start, uint8_t shutoff) {

    // Set the pump endpoints
    autoWaterStartThreshold = start;
    autoWaterShutoffThreshold = shutoff;
}

void SoilMonitor::BeginAutoWatering()   {

    // Turn on soil sensor and pump power
    digitalWrite(SOILSENSOR_PWR_PIN, HIGH);
    digitalWrite(PUMP_PWR_PIN, HIGH);

    // Map the shutoff threshold to a raw value from its percentage so that it only needs to be calculated once
    int16_t rawShutoffThreshold = map(autoWaterShutoffThreshold,0,100,minMoistureLevel,maxMoistureLevel);
    Serial.println(rawShutoffThreshold);
    // The reading value will decrease as the soil becomes more saturated, loop until shutoff threshold is reached
    while(analogRead(SOILSENSOR_DATA_PIN) > rawShutoffThreshold)    {
        
        // Drop into another loop to shut off pump and wait if the water level is too high
        if(IsPumpOverflowing())    {
            digitalWrite(PUMP_PWR_PIN,LOW);
            
            while(IsPumpOverflowing())  {
                delay(500);
            }

            // Resume pumping once overflow has stopped
            digitalWrite(PUMP_PWR_PIN, HIGH);
        }
    }
}

bool SoilMonitor::IsPumpOverflowing()   {

    // By default the float sensor will be on a pullup input and will be pulled low when overflowing (normally open circuit)
    if(digitalRead(FLOAT_SENSOR_PIN)==HIGH) {
        return false;
    }
    else    {
        return true;
    }
}