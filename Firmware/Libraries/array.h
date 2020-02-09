#pragma once

template <class T1, class T2>
class pair
{
public:
    T1 a;
    T2 b;

    pair(){};
    pair(T1 a, T2 b) : a(a), b(b){};
    pair(const pair &p1)
    {
        a = p1.a;
        b = p1.b;
    }
};

template <class T>
class Array
{
public:
    Array() {}
    Array(int size) : length(size)
    {
        data_arr = new T[length];
    }
    Array(int size, T fill) : length(size)
    {
        data_arr = new T[length];
        for (int i = 0; i < length; i++)
            data_arr[i] = fill;
    }
    Array(const Array &arr)
    {
        length = arr.length;
        data_arr = new T[length];
        for (int i = 0; i < length; i++)
            data_arr[i] = arr.data_arr[i];
    }
    ~Array()
    {
        delete[] data_arr;
    }

    int size()
    {
        return length;
    }

    T &operator[](const unsigned int index)
    {
        return data_arr[index];
    }
    T &at(int index)
    {
        return data_arr[index];
    }
    T &first()
    {
        return data_arr[0];
    }
    T &last()
    {
        return data_arr[length - 1];
    }
    T *data()
    {
        return &data_arr;
    }

    Array &push(T item)
    {
        T *temp = new T[++length];
        for (int i = 0; i < length - 1; i++)
        {
            temp[i] = data_arr[i];
        }
        temp[length - 1] = item;
        delete[] data_arr;
        data_arr = temp;
        return *this;
    }
    T pop()
    {
        T removedItem;
        T *temp = new T[--length];
        for (int i = 0; i < length; i++)
        {
            temp[i] = data_arr[i];
        }
        delete[] data_arr;
        data_arr = temp;
        return temp[length + 1];
    }

    Array &insert(unsigned int index, T item)
    {
        T *temp = new T[++length];
        for (int i = 0; i < length; i++)
        {
            if (i < index)
                temp[i] = data_arr[i];
            else if (i == index)
                temp[i] = item;
            else if (i > index)
                temp[i] = data_arr[i - 1];
        }
        delete[] data_arr;
        data_arr = temp;
        return *this;
    }
    T remove(unsigned int index)
    {
        T removedItem;
        T *temp = new T[--length];
        for (int i = 0; i < length; i++)
        {
            if (i < index)
                temp[i] = data_arr[i];
            else if (i >= index)
                temp[i] = data_arr[i + 1];

            if (i == index)
                removedItem = data_arr[i];
        }
        delete[] data_arr;
        data_arr = temp;
        return temp[index];
    }

    template <class Function>
    T &find(Function compareFunc)
    {
        for (int i = 0; i < length; i++)
        {
            if (compareFunc(data_arr[i]))
                return data_arr[i];
        }
    }
    template <class Function>
    int findIndex(Function compareFunc)
    {
        for (int i = 0; i < length; i++)
        {
            if (compareFunc(data_arr[i]))
                return i;
        }
        return -1;
    }

    void clear()
    {
        length = 0;
        delete[] data_arr;
        data_arr = nullptr;
    }

private:
    int length = 0;
    T *data_arr;
};

template <class T>
class Queue
{
public:
    Queue() {}
    Queue(int size) : length(size)
    {
        data_arr = new T[length];
    }
    Queue(const Queue &q)
    {
        length = q.length;
    }
    ~Queue()
    {
        delete[] data_arr;
    }

    int size()
    {
        return length;
    }
    Queue &put(T element)
    {
        T *temp = new T[++length];
        for (int i = 0; i < length - 1; i++)
        {
            temp[i] = data_arr[i];
        }
        temp[length - 1] = element;
        delete[] data_arr;
        data_arr = temp;
        return *this;
    }
    T get()
    {
        T *temp = new T[--length];
        for (int i = 0; i < length; i++)
        {
            temp[i] = data_arr[i + 1];
        }
        T item = data_arr[0];
        delete[] data_arr;
        data_arr = temp;
        return item;
    }

private:
    int length = 0;
    T *data_arr;
};