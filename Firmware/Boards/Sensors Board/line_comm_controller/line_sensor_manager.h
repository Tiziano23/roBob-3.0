#include <EEPROM.h>

//-- EEPROM Memory Addreses --//
#define EEPROM_MIN_CAL 0x00 /* 0x00 (00) - 0x1F (31) */
#define EEPROM_MAX_CAL 0x20 /* 0x20 (32) - 0x3F (63) */
//----------------------------//

class QTR_Controller
{
private:
    int ledPin;
    int *sensors;

    int capacitorChargeTime = 10; //µs

    float sensor_value[8];
    float calibratedMin[8];
    float calibratedMax[8];

public:
    QTR_Controller(int sensors_[], int ledPin_) : sensors(sensors_), ledPin(ledPin_) {}

    void init()
    {
        pinMode(ledPin, OUTPUT);
        for (int i = 0; i < 8; i++)
        {
            EEPROM.get(EEPROM_MIN_CAL + (i * sizeof(float)), calibratedMin[i]);
            EEPROM.get(EEPROM_MAX_CAL + (i * sizeof(float)), calibratedMax[i]);
        }
    }

    bool checkRightAngle(int dir)
    {
        bool left = true;
        bool right = true;
        for (int i = 0; i < 8; i++)
        {
            if (i < 4)
            {
                if (sensor_value[i] < 900)
                    left = false;
            }
            else
            {
                if (sensor_value[i] < 900)
                    right = false;
            }
        }
        return dir == 1 ? right : left;
    }
    int checkFull()
    {
        bool fullWhite = true;
        bool fullBlack = true;
        for (int i = 0; i < 8; i++)
        {
            if (sensor_value[i] > 200)
                fullWhite = false;
            if (sensor_value[i] < 800)
                fullBlack = false;
        }
        return (fullWhite * -1) + fullBlack;
    }
    float getLine()
    {
        measure_times();
        float vSum = 0;
        float wSum = 0;
        for (int i = 0; i < 8; i++)
        {
            vSum += sensor_value[i] * i * 1000;
            wSum += sensor_value[i];
        }
        if (checkFull() != 0)
        {
            return 3500;
        }
        if (checkRightAngle(-1))
        {
            return 0;
        }
        if (checkRightAngle(1))
        {
            return 7000;
        }
        return vSum / wSum;
    }

    void calibrate()
    {
        for (int i = 0; i < 8; i++)
        {
            calibratedMin[i] = 2000;
            calibratedMax[i] = 0;
        }
        unsigned long startTime = millis();
        while (millis() - startTime < 5000)
        {
            measure_times_raw();
            for (int i = 0; i < 8; i++)
            {
                if (sensor_value[i] > 50)
                {
                    if (sensor_value[i] < calibratedMin[i])
                        calibratedMin[i] = sensor_value[i];
                    if (sensor_value[i] > calibratedMax[i])
                        calibratedMax[i] = sensor_value[i];
                }
            }
        }
        clearTimes();
        for (int i = 0; i < 8; i++)
        {
            EEPROM.put(EEPROM_MIN_CAL + (i * sizeof(float)), calibratedMin[i]);
            EEPROM.put(EEPROM_MAX_CAL + (i * sizeof(float)), calibratedMax[i]);
        }
    }

    void clearTimes()
    {
        for (int i = 0; i < 8; i++)
        {
            sensor_value[i] = 0;
        }
    }
    void measure_times()
    {
        measure_times_raw();
        for (int i = 0; i < 8; i++)
        {
            sensor_value[i] = constrain(sensor_value[i], calibratedMin[i], calibratedMax[i]);
            sensor_value[i] = map(sensor_value[i], calibratedMin[i], calibratedMax[i], 0, 1000);
            if (sensor_value[i] < 50)
                sensor_value[i] = 0;
        }
    }
    void measure_times_raw()
    {
        digitalWrite(ledPin, HIGH); //Turn on LEDs
        delayMicroseconds(10);      // Give LEDs time to react
        for (int i = 0; i < 8; i++)
        {

            digitalWrite(sensors[i], HIGH);
            delayMicroseconds(capacitorChargeTime); // Wait for the capacitors to charge up

            pinMode(sensors[i], INPUT); // Switch to high impedence mode

            unsigned long startTime = micros();
            while (digitalRead(sensors[i]) && micros() - startTime < calibratedMin[i] + 1000)
                ;
            sensor_value[i] = (micros() - startTime);

            pinMode(sensors[i], OUTPUT); // Revert back to charge mode
            digitalWrite(sensors[i], LOW);
        }
        //Turn off LEDs
        digitalWrite(ledPin, LOW); // Turn off LEDS
    }

    void printValues()
    {
        measure_times();
        int maxIndex = 0;
        float maxVal = 0;
        for (int i = 0; i < 8; i++)
        {
            if (sensor_value[i] > maxVal)
            {
                maxVal = sensor_value[i];
                maxIndex = i;
            }
        }
        Serial.print("|");
        for (int i = 0; i < 8; i++)
        {
            if (i == maxIndex)
                Serial.print("*");
            else
                Serial.print(".");
        }
        Serial.print("| - ");
        for (int i = 0; i < 8; i++)
        {
            Serial.print(sensor_value[i]);
            if (i < 7)
                Serial.print(", ");
        }
        Serial.println();
    }
};
