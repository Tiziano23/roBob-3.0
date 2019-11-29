#pragma once
#include "utils.h"

namespace math
{

class Vector
{
private:
    int size;
    double *values;

public:
    Vector() {}
    Vector(const int size) : size(size)
    {
        double values[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = 0;
        }
    }
    Vector(const int size, double values_[]) : size(size)
    {
        double values[size];
        for (int i = 0; i < size; i++)
        {
            values[i] = values_[i];
        }
    }

    double &operator[](const unsigned int index)
    {
        return values[index];
    }

    Vector operator+(const double &scalar)
    {
        Vector t(size);
        for (int i = 0; i < size; i++)
        {
            t[i] = values[i] + scalar;
        }
        return t;
    }
    Vector operator*(const double &scalar)
    {
        Vector v(size);
        for (int i = 0; i < size; i++)
        {
            v[i] = values[i] * scalar;
        }
        return v;
    }
    Vector &operator+=(const double &scalar)
    {
        for (int i = 0; i < size; i++)
        {
            at(i) += scalar;
        }
        return *this;
    }
    Vector &operator*=(const double &scalar)
    {
        for (int i = 0; i < size; i++)
        {
            at(i) *= scalar;
        }
        return *this;
    }
    Vector operator+(const Vector &v)
    {
        if (v.size != size)
            throw;
        Vector t(size);
        for (int i = 0; i < size; i++)
        {
            t[i] = at(i) + v.values[i];
        }
        return t;
    }
    Vector operator*(const Vector &v)
    {
        if (v.size != size)
            throw;
        Vector t(size);
        for (int i = 0; i < size; i++)
        {
            t[i] = at(i) * v.values[i];
        }
        return t;
    }
    Vector &operator+=(const Vector &v)
    {
        if (v.size == size)
            for (int i = 0; i < size; i++)
            {
                values[i] += v.values[i];
            }
        return *this;
    }
    Vector &operator*=(const Vector &v)
    {
        if (v.size == size)
            for (int i = 0; i < size; i++)
            {
                values[i] *= v.values[i];
            }
        return *this;
    }

    double magnitude()
    {
        double sum = 0;
        for (int i = 0; i < size; i++)
            sum += pow(at(i), 2);
        return sqrt(sum);
    }
    double Vector::dot(Vector &v)
    {
        if (size != v.size)
            return nan;
        double result = 0;
        for (int i = 0; i < size; i++)
        {
            result += values[i] * v[i];
        }
        return result;
    }

    Vector &map(function<double, double> func)
    {
        for (int i = 0; i < size; i++)
        {
            values[i] = func(values[i]);
        }
        return *this;
    }
    Vector &normalize()
    {
        double mag = magnitude();
        if (mag > 0)
            for (int i = 0; i < size; i++)
                at(i) /= mag;
        return *this;
    }

    int getSize() { return size; }
    double *getValues() { return values; }
    double &at(int index) { return values[index]; }

    static double dot(Vector &v, Vector &w)
    {
        if (v.size != w.size)
            return nan;
        double result = 0;
        for (int i = 0; i < v.size; i++)
        {
            result += v.at(i) * w.at(i);
        }
        return result;
    }
    static Vector map(Vector &v, function<double, double> func)
    {
        Vector result(v.size);
        for (int i = 0; i < result.size; i++)
        {
            result.at(i) = func(v.at(i));
        }
        return result;
    }
};

class Matrix
{
private:
    int size;
    int rows = 0;
    int columns = 0;
    double *values;

public:
    Matrix() {}
    Matrix(const int columns, const int rows) : columns(columns), rows(rows)
    {
        size = rows * columns;
        double values[rows * columns];
        for (int i = 0; i < size; i++)
        {
            values[i] = 0;
        }
    }
    Matrix(const int columns, const int rows, double values_[]) : columns(columns), rows(rows)
    {
        size = rows * columns;
        double values[rows * columns];
        for (int i = 0; i < size; i++)
        {
            values[i] = values_[i];
        }
    }

    double &operator[](const unsigned int index)
    {
        return values[index];
    }

    Matrix Matrix::operator+(const double &scalar)
    {
        Matrix t(columns, rows);
        for (int i = 0; i < size; i++)
        {
            t[i] = at(i) + scalar;
        }
        return t;
    }
    Matrix &operator+=(const double &scalar)
    {
        for (int i = 0; i < size; i++)
        {
            at(i) += scalar;
        }
        return *this;
    }
    Matrix operator*(const double &scalar)
    {
        Matrix t(columns, rows);
        for (int i = 0; i < size; i++)
        {
            t[i] = at(i) * scalar;
        }
        return t;
    }
    Matrix &operator*=(const double &scalar)
    {
        for (int i = 0; i < size; i++)
        {
            at(i) *= scalar;
        }
        return *this;
    }

    Matrix operator+(const Matrix &m)
    {
        if (rows != m.rows || columns != m.columns)
            throw;
        Matrix t(columns, rows);
        for (int i = 0; i < size; i++)
        {
            t[i] = at(i) + m.values[i];
        }
        return t;
    }
    Matrix &operator+=(const Matrix &m)
    {
        if (rows == m.rows && columns == m.columns)
            for (int i = 0; i < size; i++)
            {
                at(i) += m.values[i];
            }
        return *this;
    }
    Matrix operator*(const Matrix &m)
    {
        if (columns != m.rows)
            throw;
        Matrix t(m.columns, rows);
        for (int i = 0; i < t.rows; i++)
        {
            for (int j = 0; j < t.columns; j++)
            {
                Vector v(columns);
                Vector w(m.rows);
                for (int k = 0; k < columns; k++)
                {
                    v[k] = at(i, k);
                    w[k] = m.values[k * m.columns + j];
                }
                t.at(i, j) = Vector::dot(v, w);
            }
        }
        return t;
    }
    Matrix &operator*=(const Matrix &m)
    {
        if (rows == m.rows && columns == m.columns)
            for (int i = 0; i < size; i++)
            {
                at(i) *= m.values[i];
            }
        return *this;
    }

    Vector operator*(Vector &v)
    {
        if (columns != v.getSize())
            throw;
        Vector t(rows);
        for (int i = 0; i < rows; i++)
        {
            Vector w(columns);
            for (int j = 0; j < columns; j++)
            {
                w[j] = at(i, j);
            }
            t[i] = Vector::dot(v, w);
        }
        return t;
    }

    Matrix &map(function<double, double> f)
    {
        for (int i = 0; i < size; i++)
        {
            at(i) = f(at(i));
        }
    }

    double &at(int index) { return values[index]; }
    double &at(int row, int column) { return values[row * columns + column]; }
    double *getValues() { return values; }

    static Matrix rowMatrixFromVector(Vector &v)
    {
        return Matrix(v.getSize(), 1, v.getValues());
    }
    static Matrix columnMatrixFromVector(Vector &v)
    {
        return Matrix(1, v.getSize(), v.getValues());
    }

    static Matrix map(Matrix &m, function<double, double> func)
    {
        Matrix result(m.columns, m.rows);
        for (int i = 0; i < result.size; i++)
        {
            result.at(i) = func(m.at(i));
        }
        return result;
    }
};

double avg(double values[], int n)
{
    float s = 0;
    for (int i = 0; i < n; i++)
    {
        s += values[i];
    }
    return s / (double)n;
}

} // namespace math