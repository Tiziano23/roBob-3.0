#pragma once

#define MATH_g 9.81 //9.80665
#define MATH_toDegrees 180/PI
#define MATH_toRadians PI/180

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

    Vector3f operator+(const Vector3f &v)
    {
        Vector3f w;
        w.x = x + v.x;
        w.y = y + v.y;
        w.z = z + v.z;
        return w;
    }
    Vector3f operator-(const Vector3f &v)
    {
        Vector3f w;
        w.x = x - v.x;
        w.y = y - v.y;
        w.z = z - v.z;
        return w;
    }
    Vector3f operator*(const double &scalar)
    {
        Vector3f w;
        w.x = x * scalar;
        w.y = y * scalar;
        w.z = z * scalar;
        return w;
    }
    Vector3f operator*(const Vector3f &v)
    {
        Vector3f w;
        w.x = x * v.x;
        w.y = y * v.y;
        w.z = z * v.z;
        return w;
    }
    Vector3f operator/(const double &scalar)
    {
        Vector3f w;
        w.x = x / scalar;
        w.y = y / scalar;
        w.z = z / scalar;
        return w;
    }
    Vector3f operator/(const Vector3f &v)
    {
        Vector3f w;
        w.x = x / v.x;
        w.y = y / v.y;
        w.z = z / v.z;
        return w;
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
    Vector3f &operator/=(const double scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
    Vector3f &operator/=(const Vector3f &v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    double dot(Vector3f &v)
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }

    double xyDist()
    {
        return sqrt(pow(x, 2) + pow(y,2));
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