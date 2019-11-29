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
    SPISettings slaveSettings = SPISettings(7500000, MSBFIRST, SPI_MODE0);
    static const byte actionBufferOffset = 1;
    static const byte dataBufferOffset = 17;
    static const int TRANSFER_DELAY = 100; //µs

public:
    void init()
    {
        SPI.begin();
    }
    template <typename T>
    T requestData(byte address)
    {
        union U {
            T value;
            byte bytes[sizeof(T)];
        } conversion;

        digitalWrite(SS, LOW);
        SPI.beginTransaction(slaveSettings);

        SPI.transfer(dataBufferOffset + address);
        delayMicroseconds(TRANSFER_DELAY);
        for (byte i = 0; i < sizeof(T) - 1; i++)
        {
            conversion.bytes[i] = SPI.transfer(dataBufferOffset + address);
            delayMicroseconds(TRANSFER_DELAY);
        }
        conversion.bytes[sizeof(T) - 1] = SPI.transfer(NONE);
        delayMicroseconds(TRANSFER_DELAY);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();

        return conversion.value;
    }
    void execAction(byte address)
    {
        digitalWrite(SS, LOW);
        SPI.beginTransaction(slaveSettings);

        SPI.transfer(actionBufferOffset + address);
        delayMicroseconds(TRANSFER_DELAY);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();
    }
};
