#pragma once
#include "libraries/math_nn.h"
using namespace math;

struct NetworkState
{
    double hB[4];
    double hW[16];
    double oB[4];
    double oW[16];
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
        hW = math::Matrix(4, 4, savedState.hW);
        oB = math::Vector(4, savedState.oB);
        oW = math::Matrix(4, 4, savedState.oW);
    }

    double *predict(double inputs[])
    {
        feedForward(inputs);
        return oA.getValues();
    }
};