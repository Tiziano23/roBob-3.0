#pragma once

#include <I2Cdev.h>
#include <MPU6050.h>

#include "libraries/math.h"
#include "libraries/utils.h"

enum KeyboardButton
{
    LEFT,
    MIDDLE,
    RIGHT
};
class Button
{
public:
    Button() {}
    Button(uint8_t pin) : pin(pin) {}
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
    uint8_t pin;
    bool once = false;
    bool repeat = false;
    bool pressed = false;
    unsigned long lastRepeat = 0;
    unsigned long lastPressed = 0;
    unsigned int repeatDeleay = 750; // ms
    unsigned int repeatPeriod = 25;  // ms
};
class Keyboard
{
public:
    Keyboard(uint8_t leftButtonPin, uint8_t centerButtonPin, uint8_t rightButtonPin, uint8_t connectedPin) : connectedPin(connectedPin)
    {
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
    uint8_t connectedPin;
};

class Buzzer
{
public:
    Buzzer(uint8_t pin_) : pin(pin_)
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
    uint8_t pin;
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
        // mpu6050.CalibrateAccel();
        // mpu6050.CalibrateGyro();
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
    SR_04(uint8_t trig_pin, uint8_t echo_pin) : trig_pin(trig_pin), echo_pin(echo_pin) {}
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
    uint8_t trig_pin;
    uint8_t echo_pin;
};

class RGBLed
{
public:
    RGBLed(uint8_t r_pin, uint8_t g_pin, uint8_t b_pin) : r_pin(r_pin), g_pin(g_pin), b_pin(b_pin)
    {
    }
    void init()
    {
        pinMode(r_pin, OUTPUT);
        pinMode(g_pin, OUTPUT);
        pinMode(b_pin, OUTPUT);
        applyColor();
    }
    void setH(double h)
    {
        color.setH(h);
        applyColor();
    }
    void setS(double s)
    {
        color.setS(s);
        applyColor();
    }
    void setV(double v)
    {
        color.setV(v);
        applyColor();
    }
    void setHSV(int h, double s, double v)
    {
        color.setHSV((float)h / 360., s, v);
        applyColor();
    }
    void setHSV(hsv data)
    {
        color.setHSV(data.h, data.s, data.v);
        applyColor();
    }
    void setRGB(double r, double g, double b)
    {
        color.setRGB(r, g, b);
        applyColor();
    }
    void setRGB(rgb data)
    {
        color.setRGB(data.r, data.g, data.b);
        applyColor();
    }
    Color &getColor() { return color; }

private:
    uint8_t r_pin;
    uint8_t g_pin;
    uint8_t b_pin;
    Color color = Color((hsv){0, 1, 0.1});

    void applyColor()
    {
        analogWrite(r_pin, color.getR() * 255);
        analogWrite(g_pin, color.getG() * 255);
        analogWrite(b_pin, color.getB() * 255);
    }
};