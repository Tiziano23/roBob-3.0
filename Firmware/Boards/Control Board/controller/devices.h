#pragma once

#include <I2Cdev.h>
#include <MPU6050.h>

#include "libraries/math.h"
#include "libraries/utils.h"

class Color
{
public:
    Color() {}
    Color(int _r, int _g, int _b)
    {
        r = constrain(_r, 0, 255);
        g = constrain(_g, 0, 255);
        b = constrain(_b, 0, 255);
        updateHSV();
    }
    Color(int _h, float _s, float _v)
    {
        h = constrain(_h, 0, 360);
        s = constrain(_s, 0.0, 1.0);
        v = constrain(_v, 0.0, 1.0);
        updateRGB();
    }

    void setR(int _r)
    {
        r = _r;
        updateHSV();
    }
    int getR() { return r; }
    void setG(int _g)
    {
        g = _g;
        updateHSV();
    }
    int getG() { return g; }
    void setB(int _b)
    {
        b = _b;
        updateHSV();
    }
    int getB() { return b; }

    void setH(float _h)
    {
        h = _h;
        updateRGB();
    }
    float getH() { return h; }
    void setS(float _s)
    {
        s = _s;
        updateRGB();
    }
    float getS() { return s; }
    void setV(float _v)
    {
        v = _v;
        updateRGB();
    }
    float getV() { return v; }

private:
    int r = 0;
    int g = 0;
    int b = 0;

    int h = 0;
    float s = 0;
    float v = 0;

    void updateRGB()
    {
        float kR = (5 + (float)h / 60.0);
        float kG = (3 + (float)h / 60.0);
        float kB = (1 + (float)h / 60.0);
        kR = kR - ((int)kR / 6) * 6;
        kG = kG - ((int)kG / 6) * 6;
        kB = kB - ((int)kB / 6) * 6;
        r = (v - v * s * max(min(min(kR, 4 - kR), 1), 0)) * 255;
        g = (v - v * s * max(min(min(kG, 4 - kG), 1), 0)) * 255;
        b = (v - v * s * max(min(min(kB, 4 - kB), 1), 0)) * 255;
    }
    void updateHSV()
    {
        v = max(max(r, g), b);
        float n = v - min(min(r, g), b);
        float k = 0;
        if (v == r)
            k = n ? (g - b / n) : 0;
        else if (v == g)
            k = n ? (2 + (b - r) / n) : 0;
        else if (v == b)
            k = n ? (4 + (r - g) / n) : 0;
        h = 60 * (k < 0 ? k + 6 : k);
        s = v ? 0 : n / v;
    }
};

class Button
{
public:
    Button() {}
    Button(int pin_) : pin(pin_) {}
    void update()
    {
        if (!digitalRead(pin))
        {
            if ((millis() - lastPressed > 65) || once)
            {
                once = !pressed;
            }
            if (once)
            {
                lastPressed = millis();
            }
            repeat = once;
            if (millis() - lastPressed > repeatDeleay && millis() - lastRepeat > repeatPeriod)
            {
                repeat = true;
                lastRepeat = millis();
            }
            pressed = true;
        }
        else
        {
            once = false;
            repeat = false;
            pressed = false;
        }
    }
    byte getPin()
    {
        return pin;
    }
    bool isPressed()
    {
        return pressed;
    }
    bool isPressedOnce()
    {
        return once;
    }
    bool isPressedRepeat()
    {
        return repeat;
    }

private:
    int pin;
    bool once = false;
    bool repeat = false;
    bool pressed = false;
    unsigned long lastRepeat = 0;
    unsigned long lastPressed = 0;
    unsigned int repeatDeleay = 750; // ms
    unsigned int repeatPeriod = 25;  // ms
};
enum KeyboardButton
{
    LEFT,
    MIDDLE,
    RIGHT
};
class Keyboard
{
public:
    Keyboard(int leftButtonPin, int centerButtonPin, int rightButtonPin, int connectedPin_)
    {
        connectedPin = connectedPin_;
        buttons[0] = leftButtonPin;
        buttons[1] = centerButtonPin;
        buttons[2] = rightButtonPin;
    }
    void init()
    {
        pinMode(connectedPin, INPUT_PULLUP);
        for (int i = 0; i < 3; i++)
        {
            pinMode(buttons[i].getPin(), INPUT_PULLUP);
        }
    }
    void update()
    {
        for (int i = 0; i < 3; i++)
        {
            buttons[i].update();
        }
    }
    bool pressed(KeyboardButton b)
    {
        return buttons[b].isPressed();
    }
    bool pressedOnce(KeyboardButton b)
    {
        return buttons[b].isPressedOnce();
    }
    bool pressedRepeat(KeyboardButton b)
    {
        return buttons[b].isPressedRepeat();
    }
    bool isConnected()
    {
        return !digitalRead(connectedPin);
    }

private:
    Button buttons[3];
    int connectedPin;
};

class Buzzer
{
public:
    Buzzer(int pin_) : pin(pin_)
    {
    }
    void init()
    {
        pinMode(pin, OUTPUT);
    }
    void actionTone()
    {
        tone(pin, 659, 50);
        delay(20);
        tone(pin, 988, 20);
    }
    void disabledTone()
    {
        tone(pin, 175, 75);
        delay(65);
        tone(pin, 82, 100);
    }

private:
    int pin;
};

class GyroscopeAccelerometer
{
public:
    GyroscopeAccelerometer() {}

    void init()
    {
        Wire.begin();
        mpu6050.initialize();
        // if (!mpu6050.testConnection()) error;
        calibrate();
    }

    void calibrate()
    {
        mpu6050.CalibrateAccel();
        mpu6050.CalibrateGyro();
    }

    void update()
    {
        deltaTime = (double)(millis() - lastUpdate) / 1000;
        lastUpdate = millis();

        gyroscope += getSensorRotationVelocity() * deltaTime;
        acceleation = getSensorAcceleration();
        rotation = calcRotation();
    }

    math::Vector3f getRotation()
    {
        return rotation;
    }

    math::Vector3f getAcceleration()
    {
        return acceleation;
    }

private:
    unsigned long long lastUpdate;
    double deltaTime = 0;

    math::Vector3f rotation;

    math::Vector3f gyroscope;
    math::Vector3f acceleation;

    MPU6050 mpu6050;

    math::Vector3f calcRotation()
    {
        math::Vector3f gyroAngle(gyroscope.x, gyroscope.y, gyroscope.z);
        math::Vector3f accelAngle;
        accelAngle.x = atan(+acceleation.x / sqrt(pow(acceleation.x, 2) + pow(acceleation.z, 2)));
        accelAngle.y = atan(-acceleation.x / sqrt(pow(acceleation.y, 2) + pow(acceleation.z, 2)));
        accelAngle.z = atan(+acceleation.z / sqrt(pow(acceleation.x, 2) + pow(acceleation.y, 2)));
        return gyroAngle * 0.96 + accelAngle * 0.04;
    }

    math::Vector3f getRawSensorRotationVelocity()
    {
        int16_t xVal, yVal, zVal;
        mpu6050.getRotation(&xVal, &yVal, &zVal);
        return math::Vector3f(xVal, yVal, zVal) / 131 * MATH_toRadians;
    }
    math::Vector3f getSensorRotationVelocity()
    {
        int16_t xVal, yVal, zVal;
        mpu6050.getRotation(&xVal, &yVal, &zVal);
        return math::Vector3f(xVal, yVal, zVal) / 131 * MATH_toRadians;
    }
    math::Vector3f getRawSensorAcceleration()
    {
        int16_t xVal, yVal, zVal;
        mpu6050.getAcceleration(&xVal, &yVal, &zVal);
        return math::Vector3f(xVal, yVal, zVal) / 16384 * MATH_g;
    }
    math::Vector3f getSensorAcceleration()
    {
        int16_t xVal, yVal, zVal;
        mpu6050.getAcceleration(&xVal, &yVal, &zVal);

        math::Vector3f g = math::Vector3f(0, 0, 1);
        math::Vector3f a = (math::Vector3f(xVal, yVal, zVal) / 16384 - g) * MATH_g;
        return a;
    }
};

class SR_04
{
public:
    SR_04(int trig_pin, int echo_pin) : trig_pin(trig_pin), echo_pin(echo_pin) {}
    void init()
    {
        pinMode(trig_pin, OUTPUT);
        pinMode(echo_pin, INPUT);
    }
    float getDist()
    {
        digitalWrite(trig_pin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig_pin, LOW);
        return pulseIn(echo_pin, HIGH) / 48.0;
    }

private:
    const unsigned int trig_pin;
    const unsigned int echo_pin;
};

class RGBLed
{
public:
    RGBLed(int r_pin, int g_pin, int b_pin) : r_pin(r_pin), g_pin(g_pin), b_pin(b_pin)
    {
    }
    void init()
    {
        pinMode(r_pin, OUTPUT);
        pinMode(g_pin, OUTPUT);
        pinMode(b_pin, OUTPUT);
        applyColor();
    }
    void setH(int h)
    {
        color.setH(h);
        applyColor();
    }
    void setS(float s)
    {
        color.setS(s);
        applyColor();
    }
    void setV(float v)
    {
        color.setV(v);
        applyColor();
    }
    void setColor(Color c) { color = c; }
    Color &getColor() { return color; }

private:
    const unsigned int r_pin;
    const unsigned int g_pin;
    const unsigned int b_pin;
    Color color;

    void applyColor()
    {
        analogWrite(r_pin, color.getR());
        analogWrite(g_pin, color.getG());
        analogWrite(b_pin, color.getB());
    }
};