/*
 *  Arduino SoilMonitor library to read 
 *  moisture data and run the auto water feature
 * 
 *  Author: Case Zuiderveld
 *  Last updated: 5/8/2023
 */

#ifndef SOILMONITOR_H
#define SOILMONITOR_H

#define DEFAULT_MIN_MOISTURE                (855)                                           // Default value can be changed with CalibrateSensor()
#define DEFAULT_MAX_MOISTURE                (490)                                           // Default value can be changed with CalibrateSensor()
#define DEFAULT_AUTOWATER_START_THRESHOLD   (35)                                            // Default value can be changed with SetPumpThresholds()
#define DEFAULT_AUTOWATER_SHUTOFF_THRESHOLD (85)                                            // Default value can be changed with SetPumpThresholds()
#define SAMPLE_QUANTITY                     (5)                                             // Number of sensor readings to average together

#include <Arduino.h>                                                                        // Required for pin read/write functions

class SoilMonitor   {
    public:
        SoilMonitor(uint8_t sensorPowerPin,                                                 //  Overloaded constructor will configure pinouts 
                    uint8_t sensorDataPin,                                                  //  when using soil sensor and autowater
                    uint8_t pumpPowerPin, 
                    uint8_t floatSensorPin);
        SoilMonitor(uint8_t sensorPowerPin,                                                 //  This constructor can be used when only using a soil sensor 
                    uint8_t sensorDataPin);                                               
        ~SoilMonitor();                                                                     //  Not doing anything currently
        void ReadSoilLevel();                                                               //  Reads value and stores a percent value in percentSoilLevel
        void CalibrateSensor(uint16_t minLevel, uint16_t maxLevel);                         //  Calibrates sensor with the min/max for different ADC/sensor/boards
        void SetAutoWaterThresholds(uint8_t start, uint8_t shutoff);                        //  Sets when to turn the pump on and off
        void BeginAutoWatering();                                                           //  Called by ReadSoilLevel if auto watering is enabled, handles running the pump until the shutoff threshold is reached
        bool IsPumpOverflowing();                                                           //  Called during autowater function to determine if pot is overflowing
        uint16_t rawSoilLevel;                                                              //  Soil level before conversion to %
        uint8_t percentSoilLevel;                                                           //  Soil level mapped to a percentage
        bool autoWater;                                                                     //  Indicates whether or not to use the auto watering feature
    
    private:
        uint8_t SOILSENSOR_PWR_PIN;                                                         //  Set by constructor
        uint8_t SOILSENSOR_DATA_PIN;                                                        //  Set by constructor
        uint8_t PUMP_PWR_PIN;                                                               //  Set by constructor
        uint8_t OVERFLOW_SENSOR_PIN;                                                        //  Set by constructor
        uint8_t FLOAT_SENSOR_PIN;                                                           //  Set by constructor
        uint16_t minMoistureLevel;                                                          //  Sensor calibration min value
        uint16_t maxMoistureLevel;                                                          //  Sensor calibration max value
        uint8_t autoWaterStartThreshold;                                                    //  Moisture level at which the pump turns on
        uint8_t autoWaterShutoffThreshold;                                                  //  Moisture level at which the pump turns off

};

#endif