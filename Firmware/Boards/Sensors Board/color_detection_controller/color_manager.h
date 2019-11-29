#pragma once

#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include "neural_network.h"

NeuralNetwork colorPredictor;
SparkFun_APDS9960 apds;

enum SensorReading
{
    GREEN,
    BLACK,
    WHITE,
    ALUMINIUM
};

class ColorSensor
{
private:
    int ledPin;
    int selectPin;
    double values[4];

    void select()
    {
        digitalWrite(selectPin, LOW);
        delayMicroseconds(10);
    }
    void deselect()
    {
        digitalWrite(selectPin, HIGH);
        delayMicroseconds(10);
    }

public:
    ColorSensor() {}
    ColorSensor(int selectPin, int ledPin) : selectPin(selectPin), ledPin(ledPin) {}
    bool init()
    {
        pinMode(ledPin, OUTPUT);
        pinMode(selectPin, OUTPUT);
        digitalWrite(ledPin, HIGH);
        digitalWrite(selectPin, HIGH);
        apds.init();
        select();
        bool enabled = apds.enableLightSensor();
        deselect();
        return enabled;
    }
    void measure()
    {
        uint16_t value;
        select();
        apds.readRedLight(value);
        values[0] = value;
        apds.readGreenLight(value);
        values[1] = value;
        apds.readBlueLight(value);
        values[2] = value;
        apds.readAmbientLight(value);
        values[3] = value;
        deselect();
    }
    SensorReading predictColor()
    {
        measure();
        double *results = colorPredictor.predict(values);
        int maxIndex = 0;
        int maxValue = 0;
        for (int i = 0; i < 4; i++)
        {
            if (results[i] > maxValue)
            {
                maxValue = results[i];
                maxIndex = i;
            }
        }
        switch (maxIndex)
        {
        case 0:
            return GREEN;
            break;
        case 1:
            return BLACK;
            break;
        case 2:
            return WHITE;
            break;
        case 3:
            return ALUMINIUM;
            break;
        }
    }
    
};

class ColorManager
{
private:
    ColorSensor sxSensor;
    ColorSensor dxSensor;

public:
    ColorManager(int left_enable, int left_led, int right_enable, int right_led)
    {
        sxSensor = ColorSensor(left_enable, left_led);
        dxSensor = ColorSensor(right_enable, right_led);
    }
    void init()
    {
        colorPredictor.init();
        sxSensor.init();
        dxSensor.init();
    }
    void measure()
    {
        sxSensor.measure();
        dxSensor.measure();
    }
    bool checkGreenSx()
    {
        return sxSensor.predictColor() == GREEN;
    }
    bool checkGreenDx()
    {
        return dxSensor.predictColor() == GREEN;
    }
    bool checkAluminium()
    {
        return sxSensor.predictColor() == ALUMINIUM || dxSensor.predictColor() == ALUMINIUM;
    }
};
