#pragma once
#include "libraries/math.h"
using namespace math;

struct NetworkState
{
    double hB[2];
    double hW[8];
    double oB[4];
    double oW[8];
};
#include "assets/state.h"

class NeuralNetwork
{
private:
    math::Vector hB;
    math::Matrix hW;
    math::Vector oB;
    math::Matrix oW;

    math::Vector oA = math::Vector(4);

    // Total network size = 88 bytes

    double (*activation)(double) = [](double x) {
        return 1.0 / (1 + exp(-x));
    };

    void feedForward(double inputs[])
    {
        Vector iA(4, inputs);
        Vector hA = (hW * iA) + hB;
        hA.map(activation);
        oA = (oW * hA) + oB;
        oA.map(activation);
    }

public:
    void init()
    {
        loadState(savedState);
    }

    void loadState(NetworkState savedState)
    {
        hB = math::Vector(4, savedState.hB);
        hW = math::Matrix(2, 4, savedState.hW);
        oB = math::Vector(2, savedState.oB);
        oW = math::Matrix(4, 2, savedState.oW);
    }

    double *predict(double inputs[])
    {
        feedForward(inputs);
        return oA.getValues();
    }
};