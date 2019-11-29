#include <SPI.h>

#define SS_INT 2

class Slave
{
private:
    volatile void (*actionBuffer[15])();
    volatile byte dataBuffer[240];
    byte actionOffset = 1;
    byte dataOffset = 16;

    static void SPI_SS_FALLING()
    {
    }

public:
    void init()
    {
        SPCR |= bit(SPE);
        SPCR |= bit(SPIE);

        pinMode(SS, INPUT);
        pinMode(MOSI, INPUT);
        pinMode(MISO, OUTPUT);
        pinMode(SS_INT, INPUT);

        attachInterrupt(digitalPinToInterrupt(SS_INT), SPI_SS_FALLING, FALLING);
    }

    template <typename T>
    void setValue(byte address, T value)
    {   
        union U {
            T value;
            byte bytes[sizeof(T)];
        } conversion;
        conversion.value = value;
        for (int i = 0; i < sizeof(T); i++) {
            dataBuffer[address + i] = conversion.bytes[i];
        }
    }
    byte getByte(byte address)
    {
        return dataBuffer[address - dataOffset];
    }
    
    template <typename Function>
    void setAction(byte address, Function function)
    {
        actionBuffer[address] = function;
    }
    void execAction(byte address) {
        actionBuffer[address - actionOffset]();
    }

    void onTransmissionCompleted(){
        byte address = SPDR;
        if (address > 0) {
            if (address < dataOffset) execAction(address);
            else SPDR = getByte(address);
        } else {
            SPDR = 0;
        }
    }
};