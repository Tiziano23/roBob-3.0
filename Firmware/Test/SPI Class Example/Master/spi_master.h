#include <SPI.h>

class Master
{
private:
    SPISettings slaveSettings = SPISettings(7500000, MSBFIRST, SPI_MODE0);
    byte dataOffset = 16;
    byte actionOffset = 1;
    int TRANSFER_DELAY = 5; //µs;
    
public:
    void init()
    {
        pinMode(SS, OUTPUT);
        digitalWrite(SS, HIGH);
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

        SPI.transfer(dataOffset + address);
        delayMicroseconds(TRANSFER_DELAY);
        for (byte i = 1; i < sizeof(T); i++) {
            conversion.bytes[i - 1] = SPI.transfer(dataOffset + address + i);
            delayMicroseconds(TRANSFER_DELAY);
        }
        conversion.bytes[sizeof(T) - 1] = SPI.transfer(0x00);
        delayMicroseconds(TRANSFER_DELAY);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();

        return conversion.value;
    }

    void execAction(byte address)
    {
        digitalWrite(SS, LOW);
        SPI.beginTransaction(slaveSettings);

        SPI.transfer(actionOffset + address);

        digitalWrite(SS, HIGH);
        SPI.endTransaction();
    }
};