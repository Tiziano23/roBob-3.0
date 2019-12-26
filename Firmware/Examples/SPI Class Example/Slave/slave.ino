#include "spi_interface_slave.h"

Slave slave;

void setup()
{
    Serial.begin(115200);
    slave.init();
    slave.setValue(0x00, 1.2345);
}

void loop()
{
}

ISR(SPI_STC_vect)
{
    slave.onTransmissionCompleted();
}