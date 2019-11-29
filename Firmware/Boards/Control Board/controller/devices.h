#pragma once

class Color
{
public:
    int r = 0;
    int g = 0;
    int b = 0;
    float a = 1;

    Color(int r = 0, int g = 0, int b = 0, float a = 1) : r(r), g(g), b(b), a(a)
    {
        r = constrain(r, 0, 255);
        g = constrain(g, 0, 255);
        b = constrain(b, 0, 255);
        a = constrain(a, 0, 1);
    };
};

class Button
{
private:
    int pin;
    bool once = false;
    bool repeat = false;
    bool pressed = false;
    unsigned long lastRepeat = 0;
    unsigned long lastPressed = 0;
    unsigned int repeatDeleay = 750;
    unsigned int repeatPeriod = 100;

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
};

class Keyboard
{
private:
    Button buttons[3];
    int connectedPin;

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
    bool pressed(int i)
    {
        return buttons[i].isPressed();
    }
    bool pressedOnce(int i)
    {
        return buttons[i].isPressedOnce();
    }
    bool pressedRepeat(int i)
    {
        return buttons[i].isPressedRepeat();
    }
    bool isConnected()
    {
        return !digitalRead(connectedPin);
    }
};

class Buzzer
{
private:
    int pin;

public:
    Buzzer(int pin_) : pin(pin_) {
    }
    void init() {
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
};

class SR_04
{
private:
    int trig;
    int echo;

public:
    SR_04(int trig_, int echo_) : trig(trig_), echo(echo_) {}
    void init()
    {
        pinMode(trig, OUTPUT);
        pinMode(echo, INPUT);
    }
    float getDist()
    {
        digitalWrite(trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig, LOW);
        return pulseIn(echo, HIGH) / 48.0;
    }
};

class RGBLed
{
private:
    int r_pin;
    int g_pin;
    int b_pin;
    Color color;

public:
    RGBLed(int r_pin, int g_pin, int b_pin) : r_pin(r_pin), g_pin(g_pin), b_pin(b_pin) {}
    void init()
    {
        pinMode(r_pin, OUTPUT);
        pinMode(g_pin, OUTPUT);
        pinMode(b_pin, OUTPUT);
        analogWrite(r_pin, 0);
        analogWrite(g_pin, 0);
        analogWrite(b_pin, 0);
    }

    void setColor(Color c)
    {
        color = c;
        analogWrite(r_pin, color.r * color.a);
        analogWrite(g_pin, color.g * color.a);
        analogWrite(b_pin, color.b * color.a);
    }
    void setR(int r)
    {
        color.r = r;
        analogWrite(r_pin, color.r * color.a);
    }
    void setG(int g)
    {
        color.g = g;
        analogWrite(g_pin, color.g * color.a);
    }
    void setB(int b)
    {
        color.b = b;
        analogWrite(b_pin, color.b * color.a);
    }
    void setA(float a)
    {
        color.a = a;
        analogWrite(r_pin, color.r * color.a);
        analogWrite(g_pin, color.g * color.a);
        analogWrite(b_pin, color.b * color.a);
    }

    Color getColor()
    {
        return color;
    }
};