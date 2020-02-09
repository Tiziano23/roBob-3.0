#pragma once

#include <Wire.h>
#include <SparkFun_APDS9960.h>

extern HardwareSerial Serial;
SparkFun_APDS9960 apds;

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

struct SensorCalibration
{
    double minH;
    double maxH;

    double minS;
    double maxS;

    double minV;
    double maxV;
};

struct SensorDataGreen
{
    double r;
    double g;
    double b;
    double minA;
    double maxA;
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
        calibrate(whiteRef, whiteRng);
    }
    void calibrateGreen()
    {
        calibrate(greenRef, greenRng);
    }
    void calibrateBlack()
    {
        calibrate(blackRef, blackRng);
    }
    void calibrateAluminium()
    {
        calibrate(alumiRef, alumiRng);
    }

    void saveCalibratoin(int offset)
    {
        eepromManager.put(offset + 0, whiteRef);
        eepromManager.put(offset + 1, greenRef);
        eepromManager.put(offset + 2, blackRef);
        eepromManager.put(offset + 3, alumiRef);
        eepromManager.put(offset + 4, whiteRng);
        eepromManager.put(offset + 5, greenRng);
        eepromManager.put(offset + 6, blackRng);
        eepromManager.put(offset + 7, alumiRng);
    }
    void loadCalibration(int offset)
    {
        eepromManager.get(offset + 0, whiteRef);
        eepromManager.get(offset + 1, greenRef);
        eepromManager.get(offset + 2, blackRef);
        eepromManager.get(offset + 3, alumiRef);
        eepromManager.get(offset + 4, whiteRng);
        eepromManager.get(offset + 5, greenRng);
        eepromManager.get(offset + 6, blackRng);
        eepromManager.get(offset + 7, alumiRng);
    }

    void printWhiteRef()
    {
        Serial.print(F("White ref: { "));
        Serial.print(F("r: "));
        Serial.print(whiteRef.r * 100, 5);
        Serial.print(F(", g: "));
        Serial.print(whiteRef.g * 100, 5);
        Serial.print(F(", b: "));
        Serial.print(whiteRef.b * 100, 5);
        Serial.print(F(", a: "));
        Serial.print(whiteRef.a * 100, 5);
        Serial.println(F(" }"));
    }
    void printWhiteRng()
    {
        Serial.print(F("White rng: { "));
        Serial.print(F("r: "));
        Serial.print(whiteRng.r * 100, 5);
        Serial.print(F(", g: "));
        Serial.print(whiteRng.g * 100, 5);
        Serial.print(F(", b: "));
        Serial.print(whiteRng.b * 100, 5);
        Serial.print(F(", a: "));
        Serial.print(whiteRng.a * 100, 5);
        Serial.println(F(" }"));
    }
    void printGreenRef()
    {
        Serial.print(F("Green ref: { "));
        Serial.print(F("r: "));
        Serial.print(greenRef.r * 100, 5);
        Serial.print(F(", g: "));
        Serial.print(greenRef.g * 100, 5);
        Serial.print(F(", b: "));
        Serial.print(greenRef.b * 100, 5);
        Serial.print(F(", a: "));
        Serial.print(greenRef.a * 100, 5);
        Serial.println(F(" }"));
    }
    void printGreenRng()
    {
        Serial.print(F("Green rng: { "));
        Serial.print(F("r: "));
        Serial.print(greenRng.r * 100, 5);
        Serial.print(F(", g: "));
        Serial.print(greenRng.g * 100, 5);
        Serial.print(F(", b: "));
        Serial.print(greenRng.b * 100, 5);
        Serial.print(F(", Min a: "));
        Serial.print(greenRng.minA * 100, 5);
        Serial.print(F(", Max a: "));
        Serial.print(greenRng.maxA * 100, 5);
        Serial.println(F(" }"));
    }

    SensorData getGreenRef()
    {
        return greenRef;
    }
    SensorDataGreen getGreenRng()
    {
        return greenRng;
    }

    void measure()
    {
        updateValues();
    }
    SensorData getValues()
    {
        return data;
    }
    SensorColor getDiscreteColor()
    {
        if (
            (whiteRef.r - whiteRng.r < data.r && data.r < whiteRef.r + whiteRng.r) &&
            (whiteRef.g - whiteRng.g < data.g && data.g < whiteRef.g + whiteRng.g) &&
            (whiteRef.b - whiteRng.b < data.b && data.b < whiteRef.b + whiteRng.b) &&
            (whiteRef.a - whiteRng.a < data.a && data.a < whiteRef.a + whiteRng.a))
            return WHITE;
        if (
            (greenRef.r - greenRng.r < data.r && data.r < greenRef.r + greenRng.r) &&
            (greenRef.g - greenRng.g < data.g && data.g < greenRef.g + greenRng.g) &&
            (greenRef.b - greenRng.b < data.b && data.b < greenRef.b + greenRng.b) &&
            (greenRef.a - greenRng.minA < data.a && data.a < greenRef.a + greenRng.maxA))
            return GREEN;
        if (
            (blackRef.r - blackRng.r < data.r && data.r < blackRef.r + blackRng.r) &&
            (blackRef.g - blackRng.g < data.g && data.g < blackRef.g + blackRng.g) &&
            (blackRef.b - blackRng.b < data.b && data.b < blackRef.b + blackRng.b) &&
            (blackRef.a - blackRng.a < data.a && data.a < blackRef.a + blackRng.a))
            return BLACK;
        if (
            (alumiRef.r - alumiRng.r < data.r && data.r < alumiRef.r + alumiRng.r) &&
            (alumiRef.g - alumiRng.g < data.g && data.g < alumiRef.g + alumiRng.g) &&
            (alumiRef.b - alumiRng.b < data.b && data.b < alumiRef.b + alumiRng.b) &&
            (alumiRef.a - alumiRng.a < data.a && data.a < alumiRef.a + alumiRng.a))
            return ALUMINIUM;

        return NOT_RECOGNIZED;
    }
    Color getColor()
    {
        return color;
    }

private:
    uint8_t ledPin;
    uint8_t selectPin;
    SensorData data;

    Color color = Color((rgb){0, 0, 0});

    unsigned int calibrationTime = 5000;
    float colorMultiplier = 5;
    float AmbientMultipGreenMin = 50;
    float AmbientMultipGreenMax = 250;

    float ambienceMultiplier = 250;
    double minimumRange = 2000. / 65535.;

    SensorData whiteRef = {0, 0, 0, 0};
    SensorData greenRef = {0, 0, 0, 0};
    SensorData blackRef = {0, 0, 0, 0};
    SensorData alumiRef = {0, 0, 0, 0};

    SensorData whiteRng = {0, 0, 0, 0};
    SensorDataGreen greenRng = {0, 0, 0, 0, 0};
    SensorData blackRng = {0, 0, 0, 0};
    SensorData alumiRng = {0, 0, 0, 0};

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

    void updateValues()
    {
        select();
        apds.readAmbientLight(data.a);
        uint16_t value;
        apds.readRedLight(value);
        data.r = (double)value / data.a;
        apds.readGreenLight(value);
        data.g = (double)value / data.a;
        apds.readBlueLight(value);
        data.b = (double)value / data.a;
        deselect();
        color.setRGB(data.r, data.g, data.b);
        // double total = data.r + data.g + data.b;
        // data.r = data.r / total;
        // data.g = data.g / total;
        // data.b = data.b / total;
    }
    void calibrate(SensorData &ref, SensorData &rng)
    {
        unsigned long startTime = millis() % 10000;
        double avgR = 0;
        double avgG = 0;
        double avgB = 0;
        double avgA = 0;
        SensorData min = {1, 1, 1, 1}, max = {0, 0, 0, 0};
        unsigned int samples = 0;
        while ((millis() % 10000) - startTime < calibrationTime)
        {
            measure();
            avgR += (double)data.r;
            avgG += (double)data.g;
            avgB += (double)data.b;
            avgA += (double)data.a;

            min.r = min(data.r, min.r);
            min.g = min(data.g, min.g);
            min.b = min(data.b, min.b);
            min.a = min(data.a, min.a);

            max.r = max(data.r, max.r);
            max.g = max(data.g, max.g);
            max.b = max(data.b, max.b);
            max.a = max(data.a, max.a);
            samples++;
        }
        ref.r = (avgR / samples);
        ref.g = (avgG / samples);
        ref.b = (avgB / samples);
        ref.a = (avgA / samples);
        // Calcolo Range colore
        rng.r = minimumRange + min(ref.r - min.r, max.r - ref.r) * colorMultiplier;
        rng.g = minimumRange + min(ref.g - min.g, max.g - ref.g) * colorMultiplier;
        rng.b = minimumRange + min(ref.b - min.b, max.b - ref.b) * colorMultiplier;
        //Ambient
        rng.a = min(ref.a - min.a, max.a - ref.a) * ambienceMultiplier;
    }
    void calibrate(SensorData &ref, SensorDataGreen &rng)
    {
        unsigned long startTime = millis() % 10000;
        double avgR = 0;
        double avgG = 0;
        double avgB = 0;
        double avgA = 0;
        SensorData min = {1, 1, 1, 1}, max = {0, 0, 0, 0};
        unsigned int samples = 0;
        while ((millis() % 10000) - startTime < calibrationTime)
        {
            measure();
            avgR += (double)data.r;
            avgG += (double)data.g;
            avgB += (double)data.b;
            avgA += (double)data.a;

            min.r = min(data.r, min.r);
            min.g = min(data.g, min.g);
            min.b = min(data.b, min.b);
            min.a = min(data.a, min.a);

            max.r = max(data.r, max.r);
            max.g = max(data.g, max.g);
            max.b = max(data.b, max.b);
            max.a = max(data.a, max.a);
            samples++;
        }
        ref.r = (avgR / samples);
        ref.g = (avgG / samples);
        ref.b = (avgB / samples);
        ref.a = (avgA / samples);
        // Calcolo Range colore
        rng.r = minimumRange + min(ref.r - min.r, max.r - ref.r) * colorMultiplier;
        rng.g = minimumRange + min(ref.g - min.g, max.g - ref.g) * colorMultiplier;
        rng.b = minimumRange + min(ref.b - min.b, max.b - ref.b) * colorMultiplier;
        //Ambient
        double minVal = constrain(min.a, ref.a - ref.a / 2, ref.a);
        double maxVal = constrain(max.a, ref.a, ref.a + ref.a / 2);
        rng.minA = min(ref.a - minVal, maxVal - ref.a) * AmbientMultipGreenMin;
        rng.maxA = min(ref.a - minVal, maxVal - ref.a) * AmbientMultipGreenMax;

        Serial.println("Ambient:");
        Serial.print("Reference: ");
        Serial.println(ref.a, 5);
        Serial.print("Min: ");
        Serial.println(min.a, 5);
        Serial.print("Max: ");
        Serial.println(max.a, 5);
        Serial.print("Delta Min: ");
        Serial.println(ref.a - min.a, 5);
        Serial.print("Delta Max: ");
        Serial.println(max.a - ref.a, 5);
        Serial.print("Range Min: ");
        Serial.println(rng.minA, 5);
        Serial.print("Range Max:");
        Serial.println(rng.maxA, 5);
    }
};

class ColorManager
{
public:
    ColorManager(int left_enable, int left_led, int right_enable, int right_led)
    {
        sxSensor = ColorSensor(left_enable, left_led);
        dxSensor = ColorSensor(right_enable, right_led);
    }
    void init()
    {
        sxSensor.init();
        dxSensor.init();

        sxSensor.lightsOn();
        dxSensor.lightsOn();

        loadCalibration();
    }
    void measure()
    {
        sxSensor.measure();
        dxSensor.measure();
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
        //delay(5000);
        Serial.println("Calibrating Dx...");
        dxSensor.calibrateGreen();
        //delay(5000);
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
        SensorData ref = sxSensor.getGreenRef();
        SensorDataGreen rng = sxSensor.getGreenRng();
        SensorData data = sxSensor.getValues();
        Serial.print(F("Sx Sensor: { "));
        Serial.print(F("r"));
        if (data.r < ref.r - rng.r || ref.r + rng.r < data.r)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.r * 100, 5);
        Serial.print(F(", g"));
        if (data.g < ref.g - rng.g || ref.g + rng.g < data.g)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.g * 100, 5);
        Serial.print(F(", b"));
        if (data.b < ref.b - rng.b || ref.b + rng.b < data.b)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.b * 100, 5);
        Serial.print(F(", a"));
        if (data.a < ref.a - rng.minA || ref.a + rng.maxA < data.a)
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
        SensorData ref = dxSensor.getGreenRef();
        SensorDataGreen rng = dxSensor.getGreenRng();
        SensorData data = dxSensor.getValues();
        Serial.print(F("Dx Sensor: { "));
        Serial.print(F("r"));
        if (data.r < ref.r - rng.r || ref.r + rng.r < data.r)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.r * 100, 5);
        Serial.print(F(", g"));
        if (data.g < ref.g - rng.g || ref.g + rng.g < data.g)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.g * 100, 5);
        Serial.print(F(", b"));
        if (data.b < ref.b - rng.b || ref.b + rng.b < data.b)
            Serial.print(F("(X)"));
        Serial.print(F(": "));
        Serial.print(data.b * 100, 5);
        Serial.print(F(", a"));
        if (data.a < ref.a - rng.minA || ref.a + rng.maxA < data.a)
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
