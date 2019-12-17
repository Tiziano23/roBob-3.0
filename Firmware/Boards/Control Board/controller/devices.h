#pragma once

class Color
{
private:
    int r = 0;
    int g = 0;
    int b = 0;

    int h = 0;
    float s = 0;
    float v = 0;

public:
    Color() {}
    Color(int _r, int _g, int _b)
    {
        r = constrain(_r, 0, 255);
        g = constrain(_g, 0, 255);
        b = constrain(_b, 0, 255);
    }
    Color(int _h, float _s, float _v)
    {
        h = constrain(_h, 0, 360);
        s = constrain(_s, 0.0, 1.0);
        v = constrain(_v, 0.0, 1.0);
    }

    void updateRGB()
    {
        float kR = (5 + (int)h / 60) % 6;
        float kG = (3 + (int)h / 60) % 6;
        float kB = (1 + (int)h / 60) % 6;
        r = v - v * s * max(min(min(kR, 4 - kR), 1), 0);
        g = v - v * s * max(min(min(kG, 4 - kG), 1), 0);
        b = v - v * s * max(min(min(kB, 4 - kB), 1), 0);
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

    // let f = (n, k = (n + h / 60) % 6) = > v - v *s *Math.max(Math.min(k, 4 - k, 1), 0);
    // return [ f(5), f(3), f(1) ];

    // let v = Math.max(r, g, b), n = v - Math.min(r, g, b);
    // let h = n && ((v == r) ? (g - b) / n : ((v == g) ? 2 + (b - r) / n : 4 + (r - g) / n));
    // return [ 60 * (h < 0 ? h + 6 : h), v &&n / v, v ];
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

    void applyColor()
    {
        analogWrite(r_pin, color.getR());
        analogWrite(g_pin, color.getG());
        analogWrite(b_pin, color.getB());
    }

public:
    RGBLed(int r_pin, int g_pin, int b_pin) : r_pin(r_pin), g_pin(g_pin), b_pin(b_pin)
    {
    }
    void init()
    {
        pinMode(r_pin, OUTPUT);
        pinMode(g_pin, OUTPUT);
        pinMode(b_pin, OUTPUT);
        digitalWrite(r_pin, LOW);
        digitalWrite(g_pin, LOW);
        digitalWrite(b_pin, LOW);
    }
    void setH(int h) { color.setH(h); }
    void setS(int s) { color.setS(s); }
    void setV(int v) { color.setV(v); }
    void setColor(Color c) { color = c; }
    Color &getColor() { return color; }
};