#include <EEPROM.h>
#include <QTRSensors.h>
#include "libraries/math.h"

// EEPROM Memory Addreses ----//
#define EEPROM_MIN_CAL_ADDR 0x00 // 0x00 (00) - 0x1F (31)
#define EEPROM_MAX_CAL_ADDR 0x20 // 0x20 (32) - 0x3F (63)
//----------------------------//

extern HardwareSerial Serial;

class QTR_Controller
{
private:
    int ledPin;
    const uint8_t *sensor_pins;

    QTRSensors qtr;

    int calibrationTime = 10000;          // ms
    int capacitorChargeTime = 10;         // µs
    int capacitorMaxDischargeTime = 1000; // µs

    uint16_t sensors_value[8];
    double sensors_min[8];
    double sensors_max[8];

    void loadCalibrationFromEEPROM()
    {
        for (int i = 0; i < 8; i++)
        {
            EEPROM.get(EEPROM_MIN_CAL_ADDR + (i * sizeof(double)), sensors_min[i]);
            EEPROM.get(EEPROM_MAX_CAL_ADDR + (i * sizeof(double)), sensors_max[i]);
        }
    }

    void saveCalibrationToEEPROM()
    {
        for (int i = 0; i < 8; i++)
        {
            EEPROM.put(EEPROM_MIN_CAL_ADDR + (i * sizeof(double)), sensors_min[i]);
            EEPROM.put(EEPROM_MAX_CAL_ADDR + (i * sizeof(double)), sensors_max[i]);
        }
    }

public:
    QTR_Controller(const uint8_t sensor_pins[], int ledPin) : sensor_pins(sensor_pins), ledPin(ledPin) {}

    void init()
    {
        // pinMode(ledPin, OUTPUT);
        // for (int i = 0; i < 8; i++)
        // {
        //     pinMode(sensor_pins[i], OUTPUT);
        // }
        // loadCalibrationFromEEPROM();

        qtr.setTypeRC();
        qtr.setSensorPins(sensor_pins, 8);
        qtr.setEmitterPin(ledPin);
        loadCalibrationFromEEPROM();
        qtr.calibrate();
        for (int i = 0; i < 8; i++)
        {
            qtr.calibrationOn.minimum[i] = sensors_min[i];
            qtr.calibrationOn.maximum[i] = sensors_max[i];
        }
    }

    bool checkRightAngle(int dir)
    {
        // bool left = true;
        // bool right = true;
        // for (int i = 0; i < 8; i++)
        // {
        //     if (i < 4)
        //     {
        //         if (sensors_value[i] < 900)
        //             left = false;
        //     }
        //     else
        //     {
        //         if (sensors_value[i] < 900)
        //             right = false;
        //     }
        // }
        // return dir == 1 ? right : left;
    }

    int checkFull()
    {
        // bool fullWhite = true;
        // bool fullBlack = true;
        // for (int i = 0; i < 8; i++)
        // {
        //     if (sensors_value[i] > 200)
        //         fullWhite = false;
        //     if (sensors_value[i] < 800)
        //         fullBlack = false;
        // }
        // return (fullWhite * -1) + fullBlack;
    }

    double getLine()
    {
        qtr.readCalibrated(sensors_value);

        bool lineDetected = false;
        for (int i = 0; i < 8; i++)
        {
            if (sensors_value[i] > 200)
            {
                lineDetected = true;
                break;
            }
        }

        if (lineDetected)
        {
            uint16_t position = qtr.readLineBlack(sensors_value);
            return (double)position / 3500 - 1;
        }
        else
            return 0;
    }

    void calibrate()
    {
        unsigned long startTime = millis();
        while (millis() - startTime < calibrationTime)
        {
            qtr.calibrate();
        }
        for (int i = 0; i < 8; i++)
        {
            sensors_min[i] = qtr.calibrationOn.minimum[i];
            sensors_max[i] = qtr.calibrationOn.maximum[i];
        }
        saveCalibrationToEEPROM();
    }

    // void readLine()
    // {
    //     measure_times();
    //     double vSum = 0;
    //     double wSum = 0;
    //     for (int i = 1; i <= 8; i++)
    //     {
    //         vSum += sensors_value[i] * i;
    //         wSum += sensors_value[i];
    //     }
    //     if (checkFull() != 0)
    //         return 0;
    //     if (checkRightAngle(-1))
    //         return -1;
    //     if (checkRightAngle(1))
    //         return +1;
    //     return vSum / wSum;
    // }
    // void calibrate()
    // {
    //     for (int i = 0; i < 8; i++)
    //     {
    //         sensors_min[i] = capacitorMaxDischargeTime;
    //         sensors_max[i] = 0;
    //     }
    //     unsigned long startTime = millis();
    //     while (millis() - startTime < calibrationTime)
    //     {
    //         measure_times_raw();
    //         for (int i = 0; i < 8; i++)
    //         {
    //             if (sensors_value[i] < sensors_min[i])
    //                 sensors_min[i] = sensors_value[i];
    //             if (sensors_value[i] > sensors_max[i])
    //                 sensors_max[i] = sensors_value[i];
    //         }
    //     }
    //     saveCalibrationToEEPROM();
    // }

    // void clearTimes()
    // {
    //     for (int i = 0; i < 8; i++)
    //     {
    //         sensors_value[i] = 0;
    //     }
    // }
    // void measure_times()
    // {
    //     measure_times_raw();
    //     for (int i = 0; i < 8; i++)
    //     {
    //         sensors_value[i] = constrain(sensors_value[i], sensors_min[i], sensors_max[i]);
    //         sensors_value[i] = math::fMap(sensors_value[i], sensors_min[i], sensors_max[i], 0, 1);
    //     }
    // }
    // void measure_times_raw()
    // {
    //     digitalWrite(ledPin, HIGH); //Turn on LEDs
    //     delayMicroseconds(10);      // Give LEDs time to react

    //     for (int i = 0; i < 8; i++)
    //     {
    //         digitalWrite(sensor_pins[i], HIGH);
    //         delayMicroseconds(capacitorChargeTime); // Wait for the capacitors to charge up

    //         pinMode(sensor_pins[i], INPUT); // Switch to high impedence mode

    //         unsigned long tStart = micros();
    //         unsigned long dischargeTime = 0;
    //         while (digitalRead(sensor_pins[i]) == HIGH && dischargeTime < capacitorMaxDischargeTime)
    //             dischargeTime = micros() - tStart;
    //         sensors_value[i] = dischargeTime;

    //         pinMode(sensor_pins[i], OUTPUT); // Revert back to charge mode
    //         digitalWrite(sensor_pins[i], LOW);
    //     }

    //     digitalWrite(ledPin, LOW); // Turn off LEDS
    // }

    void printValues()
    {
        // measure_times_raw();
        // int maxIndex = 0;
        // float maxVal = 0;
        // for (int i = 0; i < 8; i++)
        // {
        //     if (sensors_value[i] > maxVal)
        //     {
        //         maxVal = sensors_value[i];
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
            Serial.print(sensors_value[i]);
            Serial.print("|");
            Serial.print(sensors_min[i]);
            Serial.print("|");
            Serial.print(sensors_max[i]);

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
            Serial.print(sensors_min[i]);
            Serial.print("| max:");
            Serial.print(sensors_max[i]);

            if (i < 7)
                Serial.print(", ");
        }
        Serial.println();
    }
};
