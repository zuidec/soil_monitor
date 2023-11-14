#include "PlantPacket.h"

void PlantPacket::SetPlantPacketName(char *plantName)  {
  for(uint8_t i=0;i<15;i++) {
    plantName[i] = plantName[i];
  }
}

void PlantPacket::CreatePlantPacket(char* outputBuffer) {
  for(uint8_t i=0; i<15;i++)  {
    outputBuffer[i] = plantName[i];
  }
  outputBuffer[15] = (char)percentSoilLevel;
}

void PlantPacket::ParsePlantPacket(uint8_t *buffer)  {

  // Move plantname from buffer to packet one char at a time
  for(int i= 0; i < 15; i++)  {
    plantName[i] = (char)buffer[i];
  }

  // Take the last byte of the buffer and move to the packet for soil level
  percentSoilLevel = buffer[15];

  return;
}