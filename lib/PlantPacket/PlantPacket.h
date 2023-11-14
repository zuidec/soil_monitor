#ifndef PLANTPACKET_H
#define PLANTPACKET_H
#include <Arduino.h>

class PlantPacket   {
    public:
        char plantName[15];
        uint8_t percentSoilLevel;
         
        void SetPlantPacketName(char *plantName);
        void CreatePlantPacket(char* outputBuffer);
        void ParsePlantPacket(uint8_t *buffer);
    private:
};
#endif