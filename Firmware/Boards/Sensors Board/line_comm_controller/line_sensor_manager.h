#include <EEPROM.h>

//-- EEPROM Memory Addreses --//
#define EEPROM_MIN_CAL_ADDR 0x00 /* 0x00 (00) - 0x1F (31) */
#define EEPROM_MAX_CAL_ADDR 0x20 /* 0x20 (32) - 0x3F (63) */
//----------------------------//

class QTR_Controller
{
private:
    int ledPin;
    int *sensor_pins;

    int calibrationTime = 10000;          // ms
    int capacitorChargeTime = 10;         // µs
    int capacitorMaxDischargeTime = 1000; // µs

    double sensor_value[8];
    double sensor_min[8];
    double sensor_max[8];

    void loadCalibrationFromEEPROM()
    {
        for (int i = 0; i < 8; i++)
        {
            EEPROM.get(EEPROM_MIN_CAL_ADDR + (i * sizeof(double)), sensor_min[i]);
            EEPROM.get(EEPROM_MAX_CAL_ADDR + (i * sizeof(double)), sensor_max[i]);
        }
    }

    void saveCalibrationToEEPROM()
    {
        for (int i = 0; i < 8; i++)
        {
            EEPROM.put(EEPROM_MIN_CAL_ADDR + (i * sizeof(double)), sensor_min[i]);
            EEPROM.put(EEPROM_MAX_CAL_ADDR + (i * sizeof(double)), sensor_max[i]);
        }
    }

public:
    QTR_Controller(int sensor_pins[], int ledPin) : sensor_pins(sensor_pins), ledPin(ledPin) {}

    void init()
    {
        pinMode(ledPin, OUTPUT);
        for (int i = 0; i < 8; i++)
        {
            pinMode(sensor_pins[i], OUTPUT);
        }
        loadCalibrationFromEEPROM();
    }

    // bool checkRightAngle(int dir)
    // {
    //     bool left = true;
    //     bool right = true;
    //     for (int i = 0; i < 8; i++)
    //     {
    //         if (i < 4)
    //         {
    //             if (sensor_value[i] < 900)
    //                 left = false;
    //         }
    //         else
    //         {
    //             if (sensor_value[i] < 900)
    //                 right = false;
    //         }
    //     }
    //     return dir == 1 ? right : left;
    // }

    // int checkFull()
    // {
    //     bool fullWhite = true;
    //     bool fullBlack = true;
    //     for (int i = 0; i < 8; i++)
    //     {
    //         if (sensor_value[i] > 200)
    //             fullWhite = false;
    //         if (sensor_value[i] < 800)
    //             fullBlack = false;
    //     }
    //     return (fullWhite * -1) + fullBlack;
    // }

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
        // if (checkFull() != 0)
        // {
        //     return 3500;
        // }
        // if (checkRightAngle(-1))
        // {
        //     return 0;
        // }
        // if (checkRightAngle(1))
        // {
        //     return 7000;
        // }
        return vSum / wSum;
    }

    void calibrate()
    {
        for (int i = 0; i < 8; i++)
        {
            sensor_min[i] = capacitorMaxDischargeTime;
            sensor_max[i] = 0;
        }
        unsigned long startTime = millis();
        while (millis() - startTime < calibrationTime)
        {
            measure_times_raw();
            for (int i = 0; i < 8; i++)
            {
                if (sensor_value[i] < sensor_min[i])
                    sensor_min[i] = sensor_value[i];
                if (sensor_value[i] > sensor_max[i])
                    sensor_max[i] = sensor_value[i];
                printValues();
            }
        }
        printCalibrationValues();
        saveCalibrationToEEPROM();
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
            sensor_value[i] = constrain(sensor_value[i], sensor_min[i], sensor_max[i]);
            sensor_value[i] = map(sensor_value[i], sensor_min[i], sensor_max[i], 0, 1000);
        }
    }
    void measure_times_raw()
    {
        digitalWrite(ledPin, HIGH); //Turn on LEDs
        delayMicroseconds(10);      // Give LEDs time to react

        for (int i = 0; i < 8; i++)
        {
            digitalWrite(sensor_pins[i], HIGH);
            delayMicroseconds(capacitorChargeTime); // Wait for the capacitors to charge up

            pinMode(sensor_pins[i], INPUT); // Switch to high impedence mode

            unsigned long tStart = micros();
            unsigned long dischargeTime = 0;
            while (digitalRead(sensor_pins[i]) == HIGH && dischargeTime < capacitorMaxDischargeTime)
                dischargeTime = micros() - tStart;
            sensor_value[i] = dischargeTime;

            pinMode(sensor_pins[i], OUTPUT); // Revert back to charge mode
            digitalWrite(sensor_pins[i], LOW);
        }

        digitalWrite(ledPin, LOW); // Turn off LEDS
    }

    void printValues()
    {
        measure_times_raw();
        // int maxIndex = 0;
        // float maxVal = 0;
        // for (int i = 0; i < 8; i++)
        // {
        //     if (sensor_value[i] > maxVal)
        //     {
        //         maxVal = sensor_value[i];
        //         maxIndex = i;
        //     }
        // }
        // Serial.print("|");
        // for (int i = 0; i < 8; i++)
        // {
        //     if (i == maxIndex)
        //         Serial.print("*");
        //     else
        //         Serial.print(".");
        // }
        // Serial.print("| - ");
        for (int i = 0; i < 8; i++)
        {
            Serial.print(sensor_value[i]);
            Serial.print("|");
            Serial.print(sensor_min[i]);
            Serial.print("|");
            Serial.print(sensor_max[i]);

            if (i < 7)
                Serial.print(", ");
        }
        Serial.println();
    }

    void printCalibrationValues()
    {
        for (int i = 0; i < 8; i++)
        {
            Serial.print("min: ");
            Serial.print(sensor_min[i]);
            Serial.print("| max:");
            Serial.print(sensor_max[i]);

            if (i < 7)
                Serial.print(", ");
        }
        Serial.println();
    }
};
