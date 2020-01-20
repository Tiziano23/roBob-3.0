#pragma once

#include <Wire.h>
#include <SparkFun_APDS9960.h>

// #include "neural_network.h"

extern HardwareSerial Serial;

// NeuralNetwork colorPredictor;
SparkFun_APDS9960 apds;

enum SensorReading
{
    GREEN,
    BLACK,
    WHITE,
    ALUMINIUM
};

struct SensorData
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t a;
};

class ColorSensor
{
private:
    int ledPin;
    int selectPin;
    SensorData data;

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
        data.r = value;
        apds.readGreenLight(value);
        data.g = value;
        apds.readBlueLight(value);
        data.b = value;
        apds.readAmbientLight(value);
        data.a = value;
        deselect();
    }
    SensorData getReadings()
    {
        return data;
    }
    SensorReading getDiscreteColor()
    {
        if (
            (150 < data.r && data.r < 200) &&
            (220 < data.g && data.g < 300) &&
            (340 < data.b && data.b < 400) &&
            (700 < data.a && data.a < 1000))
            return WHITE;
        if (
            (20 < data.r && data.r < 40) &&
            (50 < data.g && data.g < 100) &&
            (50 < data.b && data.b < 90) &&
            (100 < data.a && data.a < 225))
            return GREEN;
        if (
            (5 < data.r && data.r < 25) &&
            (10 < data.g && data.g < 45) &&
            (15 < data.b && data.b < 55) &&
            (50 < data.a && data.a < 150))
            return BLACK;
        if (
            data.r > 230 &&
            data.g > 300 &&
            data.b > 400 &&
            data.a > 1000)
            return ALUMINIUM;
    }

    // SensorReading predictColor()
    // {
    //     measure();
    //     double *results = colorPredictor.predict(values);
    //     int maxIndex = 0;
    //     int maxValue = 0;
    //     for (int i = 0; i < 4; i++)
    //     {
    //         if (results[i] > maxValue)
    //         {
    //             maxValue = results[i];
    //             maxIndex = i;
    //         }
    //     }
    //     switch (maxIndex)
    //     {
    //     case 0:
    //         return GREEN;
    //         break;
    //     case 1:
    //         return BLACK;
    //         break;
    //     case 2:
    //         return WHITE;
    //         break;
    //     case 3:
    //         return ALUMINIUM;
    //         break;
    //     }
    // }
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
        // colorPredictor.init();
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
        return sxSensor.getDiscreteColor() == GREEN;
        // return sxSensor.predictColor() == GREEN;
    }
    bool checkGreenDx()
    {
        return dxSensor.getDiscreteColor() == GREEN;
        // return dxSensor.predictColor() == GREEN;
    }
    bool checkAluminium()
    {
        return sxSensor.getDiscreteColor() == ALUMINIUM || dxSensor.getDiscreteColor() == ALUMINIUM;
        // return sxSensor.predictColor() == ALUMINIUM || dxSensor.predictColor() == ALUMINIUM;
    }

    void printValuesSx()
    {
        Serial.print("Left Sensor: { ");
        Serial.print("r: ");
        Serial.print(sxSensor.getReadings().r);
        Serial.print(", g: ");
        Serial.print(sxSensor.getReadings().g);
        Serial.print(", b: ");
        Serial.print(sxSensor.getReadings().b);
        Serial.print(", a: ");
        Serial.print(sxSensor.getReadings().a);
        Serial.print(", green: ");
        Serial.print(checkGreenSx());
        Serial.println(" }");
    }
};
