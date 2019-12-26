#include "spi_interface_master.h"

Master master;
void setup()
{
    Serial.begin(115200);
    master.init();
}

void loop()
{
    Serial.println(master.requestData<double>(0x00), 4);
    delay(1000);
}