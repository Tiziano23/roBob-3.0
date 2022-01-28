#include <EEPROM.h>
#include <QTRSensors.h>
#include "libraries/math.h"
#include "libraries/eepromManager.h"

extern HardwareSerial Serial;

struct SensorCalibration
{
    uint16_t min = 0;
    uint16_t max = 0;
};

class QTR_Controller
{
public:
    QTR_Controller(uint8_t sensorPins[], uint8_t frontPin, uint8_t ledPin) : sensorPins(sensorPins), frontPin(frontPin), ledPin(ledPin) {}
    void init()
    {
        pinMode(frontPin, INPUT);

        qtr.setTypeRC();
        qtr.setSensorPins(sensorPins, 8);
        qtr.setEmitterPin(ledPin);
        // qtr.setTimeout(2000);

        initializeCalibrations();
        loadCalibrationsFromEEPROM();
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
        if (ready)
            return readLine();
        else
            return 0.;
    }

    void calibrate()
    {
        frontCalibration.min = 1023;
        frontCalibration.max = 0;
        qtr.resetCalibration();
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
        saveCalibrationsToEEPROM();
        ready = true;
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
    void printRawValues()
    {
        qtr.read(sensorValues);
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
        Serial.print('F');
        Serial.print(" | min: ");
        Serial.print(frontCalibration.min);
        Serial.print(", max: ");
        Serial.print(frontCalibration.max);
        Serial.println();
    }

private:
    static const int calibrationTime = 10000; // ms

    QTRSensors qtr;
    const uint8_t ledPin;
    const uint8_t frontPin;
    const uint8_t *sensorPins;

    bool ready = false;
    uint16_t sensorValues[8];
    uint16_t frontValue;
    SensorCalibration frontCalibration;

    void readFront()
    {
        frontValue = analogRead(frontPin);
    }
    void readFrontCalibrated()
    {
        frontValue = map(analogRead(frontPin), frontCalibration.min, frontCalibration.max, 0, 1000);
    }
    
    void readSafe()
    {
        qtr.read(sensorValues);
        
        // bool valid = true;
        // uint16_t val = sensorValues[0];
        // if (sensorValues[0] == val)
            
    }

    double readLine()
    {
        bool lineDetected = false;
        qtr.readCalibrated(sensorValues);
        for (int i = 0; i < 8; i++)
            lineDetected = lineDetected || (sensorValues[i] > 200);

        if (lineDetected)
        {
            // Front sensor contribution
            readFrontCalibrated();
            // sensorValues[3] += frontValue;
            // sensorValues[3] /= 2;
            // sensorValues[4] += frontValue;
            // sensorValues[4] /= 2;

            double w = 0;
            double s = 0;
            for (int i = 0; i < 8; i++)
            {
                if (sensorValues[i] > 50)
                {
                    w += sensorValues[i] * (i + 1) * 1000;
                    s += sensorValues[i];
                }
            }
            return (w / s - 1000) / 3500;
        }
        else
            return 0.;
    }

    void initializeCalibrations()
    {
        qtr.calibrationOn.minimum = (uint16_t *)realloc(qtr.calibrationOn.maximum, sizeof(uint16_t) * 8);
        qtr.calibrationOn.maximum = (uint16_t *)realloc(qtr.calibrationOn.maximum, sizeof(uint16_t) * 8);
        for (int i = 0; i < 8; i++)
        {
            qtr.calibrationOn.minimum[i] = 0;
            qtr.calibrationOn.maximum[i] = 0;
        }
        qtr.calibrationOn.initialized = true;
    }
    void loadCalibrationsFromEEPROM()
    {
        bool valid = true;
        for (int i = 0; i < 8; i++)
        {
            valid = valid && eepromManager.get(i * 2, qtr.calibrationOn.minimum[i]);
            valid = valid && eepromManager.get(i * 2 + 1, qtr.calibrationOn.maximum[i]);
        }
        valid = valid && eepromManager.get(16, frontCalibration);
        ready = valid;
    }
    void saveCalibrationsToEEPROM()
    {
        if (qtr.calibrationOn.initialized)
        {
            for (int i = 0; i < 8; i++)
            {
                eepromManager.put(i * 2, qtr.calibrationOn.minimum[i]);
                eepromManager.put(i * 2 + 1, qtr.calibrationOn.maximum[i]);
            }
        }
        eepromManager.put(16, frontCalibration);
    }
};
