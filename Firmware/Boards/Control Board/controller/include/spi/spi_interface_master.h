#pragma once
#include <SPI.h>

// MISO-MOSI Communication pins --//
//------ MEGA -------- NANO ------//
//   50 -> MISO      12 -> MISO   //
//   51 -> MOSI      11 -> MOSI   //
//   52 -> SCK       13 -> SCK    //
//   53 -> SS        10 -> SS     //
//--------------------------------//

#define NONE 0x00

class SPIMasterInterface
{
private:
    const SPISettings slaveSettings = SPISettings(1600000, MSBFIRST, SPI_MODE0);
    static const uint8_t actionBufferOffset = 1;
    static const uint8_t dataBufferOffset = 17;
    static const int TRANSFER_DELAY = 2; //ms

public:
    void init()
    {
        SPI.begin();
    }
    template <typename T>
    T requestData(uint8_t address)
    {
        T data;

        digitalWrite(SS, LOW);
        SPI.beginTransaction(slaveSettings);

        SPI.transfer(dataBufferOffset + address);
        delay(TRANSFER_DELAY);
        for (uint8_t i = 0; i < sizeof(T) - 1; i++)
        {
            ((uint8_t *)&data)[i] = SPI.transfer(dataBufferOffset + address);
            delay(TRANSFER_DELAY);
        }
        ((uint8_t *)&data)[sizeof(T) - 1] = SPI.transfer(NONE);
        delay(TRANSFER_DELAY);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();

        return data;
    }
    void execAction(uint8_t address)
    {
        digitalWrite(SS, LOW);
        SPI.beginTransaction(slaveSettings);

        SPI.transfer(actionBufferOffset + address);
        delay(TRANSFER_DELAY);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();
    }
};
