#include "PixelMask.h"
#include <math.h>

PixelMask::PixelMask(const std::string &filename, vigra::MultiArray<2, uint8_t>* image):
    _image(image)
{
    // load test image:
    vigra::ImageImportInfo imageInfo(filename.c_str());

    if(imageInfo.isGrayscale())
    {
        // instantiate array for image data
        vigra::MultiArray<2, uint8_t> imageArray(imageInfo.shape());
        // copy image data from file into array
        vigra::importImage(imageInfo, imageArray);

        _pixelMask = imageArray;
    }
    else
    {
         std::cerr << "PixelMap is not allowed to be a color image!" << std::endl;
         exit(-1);
    }

    std::pair<float, float> statistics = computeStatisticsOfPixelsWithMask(BACKGROUND);
    _backgroundMean = statistics.first;
    _backgroundVariance = statistics.second;
    std::cout << "Background statistics: mean=" << _backgroundMean << " variance=" << _backgroundVariance << std::endl;

    statistics = computeStatisticsOfPixelsWithMask(FOREGROUND);
    _foregroundMean = statistics.first;
    _foregroundVariance = statistics.second;
    std::cout << "Foreground statistics: mean=" << _foregroundMean << " variance=" << _foregroundVariance << std::endl;
}

float PixelMask::foregroundRegionPenalty(vigra::UInt8 pixelValue) const
{
    float p = (float)pixelValue; // / 255.0f;
    return powf(_foregroundMean - p, 2.0f) / (2.0f * _foregroundVariance) + logf(sqrtf(2.0f * M_PI * _foregroundVariance));
}

float PixelMask::backgroundRegionPenalty(vigra::UInt8 pixelValue) const
{
    float p = (float)pixelValue; // / 255.0f;
    return powf(_backgroundMean - p, 2.0f) / (2.0f * _backgroundVariance) + logf(sqrtf(2.0f * M_PI * _backgroundVariance));
}

bool PixelMask::pixelIsForeground(unsigned int x, unsigned int y) const
{
    return (FOREGROUND == _pixelMask(x,y));
}

bool PixelMask::pixelIsBackground(unsigned int x, unsigned int y) const
{
    return (BACKGROUND == _pixelMask(x,y));
}

std::pair<float, float> PixelMask::computeStatisticsOfPixelsWithMask(const vigra::UInt8 mask)
{
    // compute mean
    float mean = 0.0f;
    unsigned int numPixels = 0;
    for(unsigned int y = 0; y < _pixelMask.shape(1); y++)
    {
        for(unsigned int x = 0; x < _pixelMask.shape(0); x++)
        {
            if(mask == _pixelMask(x,y))
            {
                mean += (float)(*_image)(x,y); // / 255.0f;
                numPixels++;
            }
        }
    }

    mean /= numPixels;

    // compute variance
    float variance = 0.0f;
    for(unsigned int y = 0; y < _pixelMask.shape(1); y++)
    {
        for(unsigned int x = 0; x < _pixelMask.shape(0); x++)
        {
            if(mask == _pixelMask(x,y))
            {
                // variance += powf(mean - (float)(*_image)(x,y) / 255.0f, 2.0f);
                variance += powf(mean - (float)(*_image)(x,y), 2.0f);
            }
        }
    }

    variance /= (numPixels - 1);

    return std::make_pair(mean, variance);
}
