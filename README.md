## Soil Monitor

- Source code for automating soil monitoring to keep plants happy and healthy!
- Utilizes Arduino framework in PlatformIO to build ESP32 and Arduino Pro Mini  
    targets


### Arduino Soil Sensor

This part of the project utilizes an Arduino Pro Mini 3.3 and an NRF24L01+ radio  
connected to a capacitive soil moisture sensor. The microcontroller wakes up  
four hours to take a reading, turn on the radio, transmit the reading to a base  
station, then shut the radio back down and go back to sleep. 


### ESP Soil Sensor

This part uses the same basic setup as the Arduino version of the sensor, but  
transmits the soil data over WiFi directly to the database server. This makes  
the device simpler, but the ESP32 is much more power hungry than the ATMEGA328P  
and uses up the batteries much faster. Due to this, I am working to get all the  
sensors changed to the Arduino variant.


### Base Station

The base station is an ESP32 attached to an NRF24L01+ radio that checks for new  
data from the soil sensors. Once it receives a new data packet, it uploads that  
packet to the database server via WiFi. The base station uses a power supply  
rather than batteries.
