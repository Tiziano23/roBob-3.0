#include "color_manager.h"

//------ Pins ------//
#define DX_SEL 8
#define DX_LED 12
#define SX_SEL 9
#define SX_LED 13
//------------------//

//------ Communication pins ------//
#define GREEN_SX 2  //BIT_1
#define GREEN_DX 3  //BIT_2
#define ALUMINIUM 4 //BIT_3
#define CONFIG_TX 5 //BIT_4
#define CONFIG_RX 6 //BIT_5
//--------------------------------//

extern HardwareSerial Serial;
ColorManager apds_c(SX_SEL, SX_LED, DX_SEL, DX_LED);

void setup()
{
    Serial.begin(115200);
    pinMode(GREEN_SX, OUTPUT);
    pinMode(GREEN_DX, OUTPUT);
    pinMode(ALUMINIUM, OUTPUT);

    apds_c.init();

    digitalWrite(GREEN_SX, LOW);
    digitalWrite(GREEN_DX, LOW);
    digitalWrite(ALUMINIUM, LOW);
}

void loop()
{
    apds_c.measure();

    digitalWrite(GREEN_SX, apds_c.checkGreenSx());
    digitalWrite(GREEN_DX, apds_c.checkGreenDx());
    digitalWrite(ALUMINIUM, apds_c.checkAluminium());
}

// void calibrate_ISR()
// {
//     int pulse = 0;
//     unsigned int startTime = micros();
//     while (digitalRead(CAL_COLOR_PIN) && micros() - startTime < 100)
//         ;
//     pulse = micros() - startTime;
//     if (pulse < 50)
//     {
//         calibrationStep++;
//         calibrate = true;
//     }
//     else
//     {
//         calibrationStep = -1;
//         calibrate = false;
//     }
// }