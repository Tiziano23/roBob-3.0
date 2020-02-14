#include <EEPROM.h>
#include <QTRSensors.h>
#include "libraries/math.h"
#include "libraries/eepromManager.h"

extern HardwareSerial Serial;

class QTR_Controller
{
public:
    QTR_Controller(const uint8_t sensor_pins[], uint8_t ledPin) : sensor_pins(sensor_pins), ledPin(ledPin) {}

    void init()
    {
        qtr.setTypeRC();
        qtr.setSensorPins(sensor_pins, 8);
        qtr.setEmitterPin(ledPin);

        qtr.resetCalibration();
        qtr.calibrate();

        loadCalibrationFromEEPROM();
    }

    bool checkRightAngle(int dir)
    {
        // bool left = true;
        // bool right = true;
        // for (int i = 0; i < 8; i++)
        // {
        //     if (i < 4)
        //     {
        //         if (sensorValues[i] < 900)
        //             left = false;
        //     }
        //     else
        //     {
        //         if (sensorValues[i] < 900)
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
        //     if (sensorValues[i] > 200)
        //         fullWhite = false;
        //     if (sensorValues[i] < 800)
        //         fullBlack = false;
        // }
        // return (fullWhite * -1) + fullBlack;
    }

    double getLine()
    {
        qtr.readCalibrated(sensorValues);

        bool lineDetected = false;
        for (int i = 0; i < 8; i++)
        {
            if (sensorValues[i] > 200)
            {
                lineDetected = true;
                break;
            }
        }

        if (lineDetected)
        {
            uint16_t position = qtr.readLineBlack(sensorValues);
            return (double)position / 3500. - 1.;
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
        saveCalibrationToEEPROM();
    }

    void printValues()
    {
        qtr.readCalibrated(sensorValues);
        for (int i = 0; i < 8; i++)
        {
            Serial.print(sensorValues[i]);
            Serial.print(" | ");
        }
        Serial.println();
    }
    void printCalibrationValues()
    {
        for (int i = 0; i < 8; i++)
        {
            Serial.print(i);
            Serial.print(" | min: ");
            Serial.print(qtr.calibrationOn.minimum[i]);
            Serial.print(", max: ");
            Serial.print(qtr.calibrationOn.maximum[i]);
            Serial.println();
        }
    }

private:
    const uint8_t ledPin;
    const uint8_t *sensor_pins;
    QTRSensors qtr;
    uint16_t sensorValues[8];

    static const int calibrationTime = 10000; // ms

    void loadCalibrationFromEEPROM()
    {
        if (qtr.calibrationOn.initialized)
        {
            for (int i = 0; i < 8; i++)
            {
                eepromManager.get(i * 2, qtr.calibrationOn.minimum[i]);
                eepromManager.get(i * 2 + 1, qtr.calibrationOn.maximum[i]);
            }
        }
    }
    void saveCalibrationToEEPROM()
    {
        if (qtr.calibrationOn.initialized)
        {
            for (int i = 0; i < 8; i++)
            {
                eepromManager.put(i * 2, qtr.calibrationOn.minimum[i]);
                eepromManager.put(i * 2 + 1, qtr.calibrationOn.maximum[i]);
            }
        }
    }
};
