#pragma once

#include <Wire.h>
#include <SparkFun_APDS9960.h>

#include "libraries/utils.h"

extern HardwareSerial Serial;

enum SensorColor
{
    GREEN,
    BLACK,
    WHITE,
    ALUMINIUM,
    NOT_RECOGNIZED
};
struct SensorData
{
    double r;
    double g;
    double b;
    uint16_t a;
};
struct SensorTreshold
{
    double ref;
    double rng;
};

class ColorSensor
{
public:
    ColorSensor() {}
    ColorSensor(uint8_t selectPin, uint8_t ledPin) : selectPin(selectPin), ledPin(ledPin) {}

    void init()
    {
        pinMode(ledPin, OUTPUT);
        pinMode(selectPin, OUTPUT);
        digitalWrite(ledPin, LOW);
        digitalWrite(selectPin, HIGH);

        select();
        apds.init();
        apds.enableLightSensor();
        deselect();
    }

    void lightsOn()
    {
        digitalWrite(ledPin, HIGH);
    }
    void lightsOff()
    {
        digitalWrite(ledPin, LOW);
    }
    void setLightState(bool state)
    {
        digitalWrite(ledPin, state);
    }

    void calibrateWhite() {}
    void calibrateBlack() {}
    void calibrateAluminium() {}
    void saveCalibratoin() {}
    void loadCalibration() {}

    void measure()
    {
        readValues();
    }
    SensorColor getDiscreteColor()
    {
        if (greenThreshold.ref - greenThreshold.rng < color.getH() && color.getH() < greenThreshold.ref + greenThreshold.rng)
            return GREEN;
        if (blackThreshold.ref - blackThreshold.rng < values.a && values.a < blackThreshold.ref + blackThreshold.rng)
            return BLACK;
        if (alumiThreshold.ref - alumiThreshold.rng < values.a && values.a < alumiThreshold.ref + alumiThreshold.rng)
            return ALUMINIUM;
        // if (whiteThreshold.ref - whiteThreshold.rng < values.a && values.a < whiteThreshold.ref + whiteThreshold.rng)
        return WHITE;
    }

    SensorData getValues()
    {
        return values;
    }
    Color getColor()
    {
        return color;
    }

private:
    uint8_t ledPin;
    uint8_t selectPin;
    SparkFun_APDS9960 apds;

    SensorData values;
    Color color = Color((rgb){0, 0, 0});

    SensorTreshold greenThreshold = {0.45, 0.10};

    SensorTreshold blackThreshold = {7500, 7500};
    SensorTreshold whiteThreshold = {25000, 5000};
    SensorTreshold alumiThreshold = {35000, 7500};

    unsigned int calibrationTime = 5000;

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

    void readValues()
    {
        uint16_t value;
        
        select();
        apds.readAmbientLight(values.a);
        apds.readRedLight(value);
        values.r = (double)value / values.a;
        apds.readGreenLight(value);
        values.g = (double)value / values.a;
        apds.readBlueLight(value);
        values.b = (double)value / values.a;
        deselect();
        color.setRGB(values.r, values.g, values.b);
    }

    void calibrate(SensorTreshold &threshold)
    {
        unsigned long startTime = millis() % 10000;
        unsigned int samples = 0;

        double minimum = 1;
        double maximum = 0;
        while ((millis() % 10000) - startTime < calibrationTime)
        {
            measure();
            threshold.ref += values.a;
            minimum = min(values.a, minimum);
            maximum = max(values.a, maximum);
            samples++;
        }
        threshold.ref /= samples;
        threshold.rng = min(threshold.ref - minimum, maximum - threshold.ref) * 10 / (maximum - minimum);
    }
};

class ColorManager
{
public:
    ColorManager(uint8_t left_enable, uint8_t left_led, uint8_t right_enable, uint8_t right_led)
    {
        sxSensor = ColorSensor(left_enable, left_led);
        dxSensor = ColorSensor(right_enable, right_led);
    }

    void init()
    {
        sxSensor.init();
        sxSensor.lightsOn();
        dxSensor.init();
        dxSensor.lightsOn();
        loadCalibration();
    }
    void measure()
    {
        sxSensor.measure();
        dxSensor.measure();
    }

    void lightsOn()
    {
        sxSensor.lightsOn();
        dxSensor.lightsOn();
    }
    void lightsOff()
    {
        sxSensor.lightsOff();
        dxSensor.lightsOff();
    }

    bool checkGreenLeft()
    {
        return sxSensor.getDiscreteColor() == GREEN;
    }
    bool checkGreenRight()
    {
        return dxSensor.getDiscreteColor() == GREEN;
    }
    bool checkAluminium()
    {
        return sxSensor.getDiscreteColor() == ALUMINIUM || dxSensor.getDiscreteColor() == ALUMINIUM;
    }

    void calibrateWhite()
    {
        sxSensor.calibrateWhite();
        dxSensor.calibrateWhite();
        saveCalibration();
    }
    void calibrateBlack()
    {
        sxSensor.calibrateBlack();
        dxSensor.calibrateBlack();
        saveCalibration();
    }
    void calibrateAluminium()
    {
        sxSensor.calibrateAluminium();
        dxSensor.calibrateAluminium();
        saveCalibration();
    }
    void saveCalibration()
    {
        sxSensor.saveCalibratoin();
        dxSensor.saveCalibratoin();
    }
    void loadCalibration()
    {
        sxSensor.loadCalibration();
        dxSensor.loadCalibration();
    }

    ColorSensor &getLeftSensor()
    {
        return sxSensor;
    }
    ColorSensor &getRightSensor()
    {
        return dxSensor;
    }

    void printLeftSensorValues()
    {
        sxSensor.measure();
        SensorData data = sxSensor.getValues();
        Serial.print(F("Sx Sensor: { "));
        Serial.print(F("r"));
        Serial.print(F(": "));
        Serial.print(data.r * 100);
        Serial.print(F("%, g"));
        Serial.print(F(": "));
        Serial.print(data.g * 100);
        Serial.print(F("%, b"));
        Serial.print(F(": "));
        Serial.print(data.b * 100);
        Serial.print(F("%, a"));
        Serial.print(F(": "));
        Serial.print(data.a);
        Serial.println(F(" }"));
    }
    void printRightSensorValues()
    {
        dxSensor.measure();
        SensorData data = dxSensor.getValues();
        Serial.print(F("Dx Sensor: { "));
        Serial.print(F("r"));
        Serial.print(F(": "));
        Serial.print(data.r * 100);
        Serial.print(F("%, g"));
        Serial.print(F(": "));
        Serial.print(data.g * 100);
        Serial.print(F("%, b"));
        Serial.print(F(": "));
        Serial.print(data.b * 100);
        Serial.print(F("%, a"));
        Serial.print(F(": "));
        Serial.print(data.a);
        Serial.println(F(" }"));
    }

private:
    ColorSensor sxSensor;
    ColorSensor dxSensor;
};
