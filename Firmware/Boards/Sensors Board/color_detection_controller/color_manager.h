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

    void calibrateWhite()
    {
        // calibrate(whiteRef, whiteRng);
    }
    void calibrateGreen()
    {
        // calibrate(greenRef, greenRng);
    }
    void calibrateBlack()
    {
        // calibrate(blackRef, blackRng);
    }
    void calibrateAluminium()
    {
        // calibrate(alumiRef, alumiRng);
    }

    void saveCalibratoin(int offset)
    {
        // eepromManager.put(offset + 0, whiteRef);
        // eepromManager.put(offset + 1, greenRef);
        // eepromManager.put(offset + 2, blackRef);
        // eepromManager.put(offset + 3, alumiRef);
        // eepromManager.put(offset + 4, whiteRng);
        // eepromManager.put(offset + 5, greenRng);
        // eepromManager.put(offset + 6, blackRng);
        // eepromManager.put(offset + 7, alumiRng);
    }
    void loadCalibration(int offset)
    {
        // eepromManager.get(offset + 0, whiteRef);
        // eepromManager.get(offset + 1, greenRef);
        // eepromManager.get(offset + 2, blackRef);
        // eepromManager.get(offset + 3, alumiRef);
        // eepromManager.get(offset + 4, whiteRng);
        // eepromManager.get(offset + 5, greenRng);
        // eepromManager.get(offset + 6, blackRng);
        // eepromManager.get(offset + 7, alumiRng);
    }

    void measure()
    {
        readValues();
    }
    SensorColor getDiscreteColor()
    {
        // if (greenHueTresholds.min < color.getH() && color.getH < greenHueTresholds.max)
        // {

        // }

        // if (
        //     (whiteRef.r - whiteRng.r < values.r && values.r < whiteRef.r + whiteRng.r) &&
        //     (whiteRef.g - whiteRng.g < values.g && values.g < whiteRef.g + whiteRng.g) &&
        //     (whiteRef.b - whiteRng.b < values.b && values.b < whiteRef.b + whiteRng.b) &&
        //     (whiteRef.a - whiteRng.a < values.a && values.a < whiteRef.a + whiteRng.a))
        //     return WHITE;
        // if (
        //     (greenRef.r - greenRng.r < values.r && values.r < greenRef.r + greenRng.r) &&
        //     (greenRef.g - greenRng.g < values.g && values.g < greenRef.g + greenRng.g) &&
        //     (greenRef.b - greenRng.b < values.b && values.b < greenRef.b + greenRng.b) &&
        //     (greenRef.a - greenRng.minA < values.a && values.a < greenRef.a + greenRng.maxA))
        //     return GREEN;
        // if (
        //     (blackRef.r - blackRng.r < values.r && values.r < blackRef.r + blackRng.r) &&
        //     (blackRef.g - blackRng.g < values.g && values.g < blackRef.g + blackRng.g) &&
        //     (blackRef.b - blackRng.b < values.b && values.b < blackRef.b + blackRng.b) &&
        //     (blackRef.a - blackRng.a < values.a && values.a < blackRef.a + blackRng.a))
        //     return BLACK;
        // if (
        //     (alumiRef.r - alumiRng.r < values.r && values.r < alumiRef.r + alumiRng.r) &&
        //     (alumiRef.g - alumiRng.g < values.g && values.g < alumiRef.g + alumiRng.g) &&
        //     (alumiRef.b - alumiRng.b < values.b && values.b < alumiRef.b + alumiRng.b) &&
        //     (alumiRef.a - alumiRng.a < values.a && values.a < alumiRef.a + alumiRng.a))
        //     return ALUMINIUM;

        return NOT_RECOGNIZED;
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
    threshold greenTresholds = {0.4, 0.5};

    // unsigned int calibrationTime = 5000;
    // float colorMultiplier = 5;
    // float AmbientMultipGreenMin = 50;
    // float AmbientMultipGreenMax = 250;

    // float ambienceMultiplier = 250;
    // double minimumRange = 2000. / 65535.;

    // SensorData whiteRef = {0, 0, 0, 0};
    // SensorData greenRef = {0, 0, 0, 0};
    // SensorData blackRef = {0, 0, 0, 0};
    // SensorData alumiRef = {0, 0, 0, 0};

    // SensorData whiteRng = {0, 0, 0, 0};
    // SensorDataGreen greenRng = {0, 0, 0, 0, 0};
    // SensorData blackRng = {0, 0, 0, 0};
    // SensorData alumiRng = {0, 0, 0, 0};

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

    void calibrate() {}

    // void calibrate(SensorData &ref, SensorData &rng)
    // {
    //     unsigned long startTime = millis() % 10000;
    //     double avgR = 0;
    //     double avgG = 0;
    //     double avgB = 0;
    //     double avgA = 0;
    //     SensorData min = {1, 1, 1, 1}, max = {0, 0, 0, 0};
    //     unsigned int samples = 0;
    //     while ((millis() % 10000) - startTime < calibrationTime)
    //     {
    //         measure();
    //         avgR += (double)values.r;
    //         avgG += (double)values.g;
    //         avgB += (double)values.b;
    //         avgA += (double)values.a;

    //         min.r = min(values.r, min.r);
    //         min.g = min(values.g, min.g);
    //         min.b = min(values.b, min.b);
    //         min.a = min(values.a, min.a);

    //         max.r = max(values.r, max.r);
    //         max.g = max(values.g, max.g);
    //         max.b = max(values.b, max.b);
    //         max.a = max(values.a, max.a);
    //         samples++;
    //     }
    //     ref.r = (avgR / samples);
    //     ref.g = (avgG / samples);
    //     ref.b = (avgB / samples);
    //     ref.a = (avgA / samples);
    //     // Calcolo Range colore
    //     rng.r = minimumRange + min(ref.r - min.r, max.r - ref.r) * colorMultiplier;
    //     rng.g = minimumRange + min(ref.g - min.g, max.g - ref.g) * colorMultiplier;
    //     rng.b = minimumRange + min(ref.b - min.b, max.b - ref.b) * colorMultiplier;
    //     //Ambient
    //     rng.a = min(ref.a - min.a, max.a - ref.a) * ambienceMultiplier;
    // }
    // void calibrate(SensorData &ref, SensorDataGreen &rng)
    // {
    //     unsigned long startTime = millis() % 10000;
    //     double avgR = 0;
    //     double avgG = 0;
    //     double avgB = 0;
    //     double avgA = 0;
    //     SensorData min = {1, 1, 1, 1}, max = {0, 0, 0, 0};
    //     unsigned int samples = 0;
    //     while ((millis() % 10000) - startTime < calibrationTime)
    //     {
    //         measure();
    //         avgR += (double)values.r;
    //         avgG += (double)values.g;
    //         avgB += (double)values.b;
    //         avgA += (double)values.a;

    //         min.r = min(values.r, min.r);
    //         min.g = min(values.g, min.g);
    //         min.b = min(values.b, min.b);
    //         min.a = min(values.a, min.a);

    //         max.r = max(values.r, max.r);
    //         max.g = max(values.g, max.g);
    //         max.b = max(values.b, max.b);
    //         max.a = max(values.a, max.a);
    //         samples++;
    //     }
    //     ref.r = (avgR / samples);
    //     ref.g = (avgG / samples);
    //     ref.b = (avgB / samples);
    //     ref.a = (avgA / samples);
    //     // Calcolo Range colore
    //     rng.r = minimumRange + min(ref.r - min.r, max.r - ref.r) * colorMultiplier;
    //     rng.g = minimumRange + min(ref.g - min.g, max.g - ref.g) * colorMultiplier;
    //     rng.b = minimumRange + min(ref.b - min.b, max.b - ref.b) * colorMultiplier;
    //     //Ambient
    //     double minVal = constrain(min.a, ref.a - ref.a / 2, ref.a);
    //     double maxVal = constrain(max.a, ref.a, ref.a + ref.a / 2);
    //     rng.minA = min(ref.a - minVal, maxVal - ref.a) * AmbientMultipGreenMin;
    //     rng.maxA = min(ref.a - minVal, maxVal - ref.a) * AmbientMultipGreenMax;

    //     Serial.println("Ambient:");
    //     Serial.print("Reference: ");
    //     Serial.println(ref.a, 5);
    //     Serial.print("Min: ");
    //     Serial.println(min.a, 5);
    //     Serial.print("Max: ");
    //     Serial.println(max.a, 5);
    //     Serial.print("Delta Min: ");
    //     Serial.println(ref.a - min.a, 5);
    //     Serial.print("Delta Max: ");
    //     Serial.println(max.a - ref.a, 5);
    //     Serial.print("Range Min: ");
    //     Serial.println(rng.minA, 5);
    //     Serial.print("Range Max: ");
    //     Serial.println(rng.maxA, 5);
    // }
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

    bool checkGreenSx()
    {
        return sxSensor.getDiscreteColor() == GREEN;
    }
    bool checkGreenDx()
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
    void calibrateGreen()
    {
        Serial.println("Calibrating Sx...");
        sxSensor.calibrateGreen();
        Serial.println("Calibrating Dx...");
        dxSensor.calibrateGreen();
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
        sxSensor.saveCalibratoin(0);
        dxSensor.saveCalibratoin(8);
    }
    void loadCalibration()
    {
        sxSensor.loadCalibration(0);
        dxSensor.loadCalibration(8);
    }

    ColorSensor &getLeftSensor()
    {
        return sxSensor;
    }
    ColorSensor &getRightSensor()
    {
        return dxSensor;
    }

    void printSxValues()
    {
        sxSensor.measure();
        // SensorData ref = sxSensor.getGreenRef();
        // SensorDataGreen rng = sxSensor.getGreenRng();
        SensorData data = sxSensor.getValues();
        Serial.print(F("Sx Sensor: { "));
        Serial.print(F("r"));
        // if (data.r < ref.r - rng.r || ref.r + rng.r < data.r)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.r * 100, 5);
        // Serial.print(F(", g"));
        // if (data.g < ref.g - rng.g || ref.g + rng.g < data.g)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.g * 100, 5);
        // Serial.print(F(", b"));
        // if (data.b < ref.b - rng.b || ref.b + rng.b < data.b)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.b * 100, 5);
        // Serial.print(F(", a"));
        // if (data.a < ref.a - rng.minA || ref.a + rng.maxA < data.a)
        Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.a * 100, 5);
        Serial.print(F(", green_detected: "));
        Serial.print(checkGreenSx());
        Serial.println(F(" }"));
    }
    void printDxValues()
    {
        dxSensor.measure();
        // SensorData ref = dxSensor.getGreenRef();
        // SensorDataGreen rng = dxSensor.getGreenRng();
        SensorData data = dxSensor.getValues();
        Serial.print(F("Dx Sensor: { "));
        Serial.print(F("r"));
        // if (data.r < ref.r - rng.r || ref.r + rng.r < data.r)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.r * 100, 5);
        // Serial.print(F(", g"));
        // if (data.g < ref.g - rng.g || ref.g + rng.g < data.g)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.g * 100, 5);
        // Serial.print(F(", b"));
        // if (data.b < ref.b - rng.b || ref.b + rng.b < data.b)
        //     Serial.print(F("(X)"));
        // Serial.print(F(": "));
        // Serial.print(data.b * 100, 5);
        // Serial.print(F(", a"));
        // if (data.a < ref.a - rng.minA || ref.a + rng.maxA < data.a)
        Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.a * 100, 5);
        Serial.print(F(", green_detected: "));
        Serial.print(checkGreenDx());
        Serial.println(F(" }"));
    }

private:
    ColorSensor sxSensor;
    ColorSensor dxSensor;
};
