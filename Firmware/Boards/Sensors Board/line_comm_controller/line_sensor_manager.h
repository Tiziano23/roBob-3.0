#include <EEPROM.h>
#include <QTRSensors.h>
#include "libraries/math.h"
#include "libraries/eepromManager.h"

extern HardwareSerial Serial;

class QTR_Controller
{
public:
    QTR_Controller(uint8_t sensorPins[], uint8_t ledPin, uint8_t frontPin) : sensorPins(sensorPins), ledPin(ledPin), frontPin(frontPin) {}
    ~QTR_Controller()
    {
        delete[] sensorValues;
    }

    void init()
    {
        pinMode(frontPin, INPUT);
        qtr.setTypeRC();
        qtr.setSensorPins(sensorPins, sensorCount);
        qtr.setEmitterPin(ledPin);

        qtr.resetCalibration();
        qtr.calibrate();

        loadCalibrationFromEEPROM();
        sensorValues = new uint16_t[sensorCount];
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
        readFrontCalibrated();

        bool lineDetected = false;
        for (int i = 0; i < 8; i++)
        {
            if (sensorValues[i] > 200)
            {
                lineDetected = true;
                break;
            }
            if (i == 3 || i == 4)
            {
                sensorValues[i] += frontValue;
                sensorValues[i] /= 2;
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
            readFront();
            if (frontValue < frontCalibration.min)
                frontCalibration.min = frontValue;
            if (frontValue > frontCalibration.max)
                frontCalibration.max = frontValue;
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
    struct SensorCalibration
    {
        uint16_t min;
        uint16_t max;
    };

    const uint8_t ledPin;
    const uint8_t frontPin;
    const uint8_t sensorCount = 8;
    const uint8_t *sensorPins;

    QTRSensors qtr;
    uint16_t *sensorValues;
    uint16_t frontValue;
    SensorCalibration frontCalibration = {0, 0};

    static const int calibrationTime = 10000; // ms

    void readFront()
    {
        frontValue = analogRead(frontPin);
    }
    void readFrontCalibrated()
    {
        frontValue = map(analogRead(frontPin), frontCalibration.min, frontCalibration.max, 0, 1000);
    }

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
        eepromManager.get(sensorCount * 2, frontCalibration);
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
        eepromManager.put(sensorCount * 2, frontCalibration);
    }
};
