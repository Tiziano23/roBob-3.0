#pragma once

#define MATH_g 9.81

namespace math
{

class Vector3f
{
public:
    double x;
    double y;
    double z;

    Vector3f()
    {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3f(double x, double y, double z) : x(x), y(y), z(z) {}

    Vector3f operator+(Vector3f v, const Vector3f &w)
    {
        Vector3f r;
        r.x = x + v.x;
        r.y = y + v.y;
        r.z = z + v.z;
        return r;
    }
    Vector3f operator-(Vector3f v, const Vector3f &w)
    {
        Vector3f r;
        r.x = x - v.x;
        r.y = y - v.y;
        r.z = z - v.z;
        return r;
    }
    Vector3f operator*(Vector3f v, const double &scalar)
    {
        Vector3f r;
        r.x = x * scalar;
        r.y = y * scalar;
        r.z = z * scalar;
        return r;
    }
    Vector3f operator*(Vector3f v, const Vector3f &w)
    {
        Vector3f r;
        r.x = x * v.x;
        r.y = y * v.y;
        r.z = z * v.z;
        return r;
    }

    Vector3f &operator+=(const Vector3f &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    Vector3f &operator-=(const Vector3f &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    Vector3f &operator*=(const double scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    Vector3f &operator*=(const Vector3f &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    double dot(Vector3f &v)
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }
};

int sign(double x)
{
    return x > 0 ? 1 : -1;
}

double avg(double values[], int n)
{
    float s = 0;
    for (int i = 0; i < n; i++)
    {
        s += values[i];
    }
    return s / (double)n;
}
double fMap(double n, double in_min, double in_max, double out_min, double out_max)
{
    return (n - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

} // namespace math