#include "ImageGraph.h"
#include <chrono>

ImageGraph::ImageGraph(const std::string &imageFilename, const std::string &maskFilename):
    _imageArray(),
    _graph(),
    _costs(_graph),
    _coordinates(_graph),
    _maxBoundaryPenalty(0.0f),
    _lambda(1.0f),
    _sigma(1.0f)
{
    loadImage(imageFilename);
    _pixelMask = PixelMask(maskFilename, &_imageArray);
}


ImageGraph::~ImageGraph() {}

void ImageGraph::loadImage(const std::string& filename)
{
    auto start = std::chrono::high_resolution_clock::now();
    // load test image:
    vigra::ImageImportInfo imageInfo(filename.c_str());

    if(imageInfo.isGrayscale())
    {
        // instantiate array for image data
        vigra::MultiArray<2, uint8_t> imageArray(imageInfo.shape());
        // copy image data from file into array
        vigra::importImage(imageInfo, imageArray);

        _imageArray = imageArray;
    }
    else
    {
         std::cerr << "Color conversion: Not yet implemented!" << std::endl;
         exit(-1);
    }

    std::cout << "Found Image: (" << imageInfo.width() << "x" << imageInfo.height() << ")" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "== Elapsed time: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;
}


float ImageGraph::sigma() const
{
    return _sigma;
}

void ImageGraph::setSigma(float sigma)
{
    _sigma = sigma;
}

float ImageGraph::lambda() const
{
    return _lambda;
}

void ImageGraph::setLambda(float lambda)
{
    _lambda = lambda;
}
