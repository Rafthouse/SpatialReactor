#pragma once

#include <cmath>

class DCBlocker
{
public:
    void prepare (double sampleRate)
    {
        float fc = 5.0f;
        pole = 1.0f - (2.0f * 3.14159265f * fc / (float) sampleRate);
        reset();
    }

    void reset()
    {
        x1 = 0.f;
        y1 = 0.f;
    }

    float process (float x)
    {
        float y = x - x1 + pole * y1;
        x1 = x;
        y1 = y;
        return y;
    }

    void processBlock (float* data, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
            data[i] = process (data[i]);
    }

private:
    float pole = 0.9999f;
    float x1 = 0.f, y1 = 0.f;
};
