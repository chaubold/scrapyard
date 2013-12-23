#ifndef IMAGEGRAPHDUAL_H
#define IMAGEGRAPHDUAL_H

#include "ImageGraph.h"

class ImageGraphDual : public ImageGraph
{
public:
    ImageGraphDual(const std::string& imageFilename, const std::string& maskFilename);
    virtual ~ImageGraphDual();

    virtual void buildGraph();
    virtual ImageArray runMinCut();
};

#endif // IMAGEGRAPHDUAL_H
