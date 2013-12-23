#include "ImageGraphDual.h"

ImageGraphDual::ImageGraphDual(const std::string &imageFilename, const std::string &maskFilename):
    ImageGraph(imageFilename, maskFilename)
{
}

ImageGraphDual::~ImageGraphDual() {}

void ImageGraphDual::buildGraph()
{
    // TODO: implement me
}

ImageGraph::ImageArray ImageGraphDual::runMinCut()
{
    // TODO: implement me
}
