#include "math.h"

#define DATA_OFFSET 0x1AF

struct NetworkState
{
    double hB[16];
    double hW[4*16];
    double oB[4];
    double oW[4*16];
};

class NeuralNetwork
{
private:
    math::Vector iA = math::Vector(4);

    math::Vector hA = math::Vector(16);
    math::Vector hB = math::Vector(16);
    math::Matrix hW = math::Matrix(4, 16);

    math::Vector oA = math::Vector(4);
    math::Vector oB = math::Vector(4);
    math::Matrix oW = math::Matrix(16, 4);

    // Total network size = 592 bytes

    double (*activation)(double) = [](double x) {
        return 1.0 / (1 + exp(-x));
    };

    void feedForward(double inputs[])
    {
        iA = math::Vector(iA.getSize(), inputs);

        hA = math::Matrix::multiply(&hW, &iA);
        hA.add(&hB);
        hA.map(activation);

        oA = math::Matrix::multiply(&oW, &hA);
        oA.add(&oB);
        oA.map(activation);
    }

public:
    void init()
    {
        NetworkState savedState;
        EEPROM.get(DATA_OFFSET, savedState);
        loadState(savedState);
    }

    void loadState(NetworkState savedState)
    {
        hB = math::Vector(16, savedState.hB);
        hW = math::Matrix(4, 16, savedState.hW);
        oB = math::Vector(4, savedState.oB);
        oW = math::Matrix(16, 4, savedState.oW);
    }

    double *predict(double inputs[])
    {
        feedForward(inputs);
        return oA.getValues();
    }
};