#ifndef PIXELMASK_H
#define PIXELMASK_H

#include <iostream>
#include <vigra/multi_array.hxx>
#include <vigra/impex.hxx>

class PixelMask
{
public:
    typedef enum {
        NONE = 0,
        BACKGROUND = 128,
        FOREGROUND = 255
    } PixelType;

    PixelMask() {}
    PixelMask(const std::string& filename, vigra::MultiArray<2, uint8_t> *image);

    const int probabilityOfBeingForeground(vigra::UInt8 pixelValue) const;
    const int probabilityOfBeingBackground(vigra::UInt8 pixelValue) const;

    const bool pixelIsForeground(unsigned int x, unsigned int y) const;
    const bool pixelIsBackground(unsigned int x, unsigned int y) const;

private:
    std::pair<float, float> computeStatisticsOfPixelsWithMask(const vigra::UInt8 mask);

private:
    vigra::MultiArray<2, vigra::UInt8> _pixelMask;
    vigra::MultiArray<2, vigra::UInt8>* _image;

    float _backgroundMean;
    float _backgroundVariance;

    float _foregroundMean;
    float _foregroundVariance;
};

#endif // PIXELMASK_H
