#pragma once

template <class A, class B>
using function = A (*)(B);
typedef void (*void_function)();

template <typename Function>
void repeatFor(int duration, Function action)
{
    unsigned long startTime = millis();
    while (millis() - startTime < duration)
    {
        action(millis() - startTime);
    }
}

struct rgb
{
    double r;
    double g;
    double b;
};
struct hsv
{
    double h;
    double s;
    double v;
};

class Color
{
public:
    Color() {}
    Color(rgb data)
    {
        cRGB.r = constrain(data.r, 0., 1.);
        cRGB.g = constrain(data.g, 0., 1.);
        cRGB.b = constrain(data.b, 0., 1.);
        updateHSV();
    }
    Color(hsv data)
    {
        cHSV.h = constrain(data.h, 0., 1.);
        cHSV.s = constrain(data.s, 0., 1.);
        cHSV.v = constrain(data.v, 0., 1.);
        updateRGB();
    }
    Color(const Color &c)
    {
        cRGB = c.cRGB;
        cHSV = c.cHSV;
    }

    rgb getRGB()
    {
        return cRGB;
    }
    void setRGB(double r, double g, double b)
    {
        cRGB.r = constrain(r, 0., 1.);
        cRGB.g = constrain(g, 0., 1.);
        cRGB.b = constrain(b, 0., 1.);
        updateHSV();
    }
    hsv getHSV()
    {
        return cHSV;
    }
    void setHSV(double h, float s, float v)
    {
        cHSV.h = constrain(h, 0., 1.);
        cHSV.s = constrain(s, 0., 1.);
        cHSV.v = constrain(v, 0., 1.);
        updateRGB();
    }

    double getR() { return cRGB.r; }
    void setR(double r)
    {
        cRGB.r = r;
        updateHSV();
    }
    double getG() { return cRGB.g; }
    void setG(double g)
    {
        cRGB.g = g;
        updateHSV();
    }
    double getB() { return cRGB.b; }
    void setB(double b)
    {
        cRGB.b = b;
        updateHSV();
    }
    double getH() { return cHSV.h; }
    void setH(double h)
    {
        cHSV.h = h;
        updateRGB();
    }
    double getS() { return cHSV.s; }
    void setS(double s)
    {
        cHSV.s = s;
        updateRGB();
    }
    double getV() { return cHSV.v; }
    void setV(double v)
    {
        cHSV.v = v;
        updateRGB();
    }

private:
    rgb cRGB = {0, 0, 0};
    hsv cHSV = {0, 0, 0};

    void updateRGB()
    {
        double kR = (5 + (cHSV.h * 6));
        double kG = (3 + (cHSV.h * 6));
        double kB = (1 + (cHSV.h * 6));
        kR = kR - ((int)kR / 6) * 6;
        kG = kG - ((int)kG / 6) * 6;
        kB = kB - ((int)kB / 6) * 6;
        cRGB.r = (cHSV.v - cHSV.v * cHSV.s * max(min(min(kR, 4 - kR), 1), 0));
        cRGB.g = (cHSV.v - cHSV.v * cHSV.s * max(min(min(kG, 4 - kG), 1), 0));
        cRGB.b = (cHSV.v - cHSV.v * cHSV.s * max(min(min(kB, 4 - kB), 1), 0));
    }
    void updateHSV()
    {
        cHSV.v = max(max(cRGB.r, cRGB.g), cRGB.b);
        double n = cHSV.v - min(min(cRGB.r, cRGB.g), cRGB.b);
        double k = 0;
        if (cHSV.v == cRGB.r)
            k = n != 0 ? 0 + (cRGB.g - cRGB.b) / n : 0;
        else if (cHSV.v == cRGB.g)
            k = n != 0 ? 2 + (cRGB.b - cRGB.r) / n : 0;
        else if (cHSV.v == cRGB.b)
            k = n != 0 ? 4 + (cRGB.r - cRGB.g) / n : 0;
        cHSV.h = (k < 0 ? k + 6 : k) / 60.;
        cHSV.s = cHSV.v != 0 ? n / cHSV.v : 0;
    }
};