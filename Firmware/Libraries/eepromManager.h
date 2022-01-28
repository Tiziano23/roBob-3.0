#pragma once

#include <EEPROM.h>
#include "array.h"

extern HardwareSerial Serial;

class EEPROMManager
{
public:
    EEPROMManager() {}

    void init()
    {
        loadState();
    }

    template <class T>
    void put(T data)
    {
        uint8_t id = addressIdMap.size();
        addressIdMap.push(pair<int, uint8_t>(dataPointer, id));
        EEPROM.put(dataPointer, (uint8_t)sizeof(T));
        dataPointer += sizeof(uint8_t);
        EEPROM.put(dataPointer, data);
        dataPointer += sizeof(T);
        EEPROM.put(dataPointer, id);
        dataPointer += sizeof(uint8_t);
    }
    template <class T>
    void put(uint8_t id, T data)
    {
        int index = addressIdMap.findIndex([&](pair<int, uint8_t> p) { return p.b == id; });
        if (index > -1)
        {
            if (sizeof(T) != getSize(addressIdMap[index]))
                update(id, sizeof(T) - getSize(addressIdMap[index]));
            EEPROM.put(addressIdMap[index].a + sizeof(uint8_t), data);
        }
        else
        {
            addressIdMap.push(pair<int, uint8_t>(dataPointer, id));
            EEPROM.put(dataPointer, (uint8_t)sizeof(T));
            dataPointer += sizeof(uint8_t);
            EEPROM.put(dataPointer, data);
            dataPointer += sizeof(T);
            EEPROM.put(dataPointer, id);
            dataPointer += sizeof(uint8_t);
        }
    }

    template <class T>
    T get(uint8_t id, bool &valid)
    {
        int index = addressIdMap.findIndex([&](pair<int, uint8_t> p) { return p.b == id; });
        valid = index != -1;
        if (valid)
        {
            T data;
            EEPROM.get(addressIdMap[index].a + sizeof(uint8_t), data);
            return data;
        }
        else
            return nullptr;
    }
    template <class T>
    bool get(uint8_t id, T &data)
    {
        int index = addressIdMap.findIndex([&](pair<int, uint8_t> p) { return p.b == id; });
        if (index != -1)
            EEPROM.get(addressIdMap[index].a + sizeof(uint8_t), data);
        return index != -1;
    }

    void clear()
    {
        dataPointer = 0;
        addressIdMap.clear();
        for (int i = 0; i < EEPROM.length(); i++)
        {
            EEPROM.update(i, 0);
        }
    }

private:
    int dataPointer = 0;
    Array<pair<int, uint8_t>> addressIdMap;

    void loadState()
    {
        int readingOffset = 0;
        uint8_t dataSize = 0;

        do
        {
            int address = readingOffset;
            EEPROM.get(readingOffset, dataSize);
            if (dataSize != 0)
            {
                uint8_t id;
                readingOffset += sizeof(uint8_t) + dataSize;
                EEPROM.get(readingOffset, id);
                readingOffset += sizeof(uint8_t);
                addressIdMap.push(pair<int, uint8_t>(address, id));
            }
        } while (dataSize != 0);

        dataPointer = readingOffset;
    }

    void update(int index, uint8_t sizeIncrement)
    {
        uint8_t id;
        int size = getSize(addressIdMap[index]);
        int address = addressIdMap[index].a;
        EEPROM.put(address, size + sizeIncrement);
        EEPROM.get(address + sizeof(uint8_t) + size, id);
        EEPROM.put(address + sizeof(uint8_t) + size + sizeIncrement, id);
        for (int i = address + size + 2 * sizeof(uint8_t); i < dataPointer; i++)
        {
            uint8_t byte = EEPROM.read(i);
            EEPROM.write(i + sizeIncrement, byte);
        }
        for (int i = 0; i < addressIdMap.size(); i++)
        {
            if (addressIdMap[i].a > address)
                addressIdMap[i].a += sizeIncrement;
        }
        dataPointer += sizeIncrement;
    }

    uint8_t getSize(pair<int, uint8_t> item)
    {
        uint8_t size;
        EEPROM.get(item.a, size);
        return size;
    }

    // template <class T>
    // int writeArray(int address, Array<T> &array)
    // {
    //     if (array.size() == 0)
    //         return address;
    //     else
    //     {
    //         EEPROM.put(address, array.size());
    //         for (int i = 0; i < array.size(); i++)
    //         {
    //             EEPROM.put(address + sizeof(int) + sizeof(T) * i, array[i]);
    //         }
    //         return address + sizeof(int) + sizeof(T) * array.size();
    //     }
    // }

    // template <class T>
    // int readArray(int address, Array<T> &array)
    // {
    //     int size;
    //     EEPROM.get(address, size);
    //     if (size == 0)
    //         return address;
    //     else
    //     {
    //         for (int i = 0; i < size; i++)
    //         {
    //             T data;
    //             EEPROM.get(address + sizeof(int) + sizeof(T) * i, data);
    //             array.push(data);
    //         }
    //         return address + sizeof(int) + sizeof(T) * array.size();
    //     }
    // }
};

static EEPROMManager eepromManager;