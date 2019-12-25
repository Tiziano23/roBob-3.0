#pragma once

template <class A, class B>
using function = A (*)(B);
typedef void (*void_function)();

void repeatFor(int duration, function<void, int> action)
{
    unsigned long startTime = millis();
    while (millis() - startTime < duration)
    {
        action(millis() - startTime);
    }
}

template <class T>
class Array
{
private:
    int length;
    T *data;

public:
    Array() {}
    Array(int size) : length(size)
    {
        data = new T[length];
    }
    Array(int size, T fill) : length(size)
    {
        data = new T[length];
        for (int i = 0; i < length; i++)
            data[i] = fill;
    }

    int size()
    {
        return length;
    }

    T &operator[](const unsigned int index)
    {
        return data[index];
    }
    T &at(int index)
    {
        return data[index];
    }

    Array &add(T item)
    {
        length++;
        T *temp = data;
        data = new T[length];
        for (int i = 0; i < length - 1; i++)
        {
            data[i] = temp[i];
        }
        data[length - 1] = item;
        return *this;
    }
};
