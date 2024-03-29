#ifndef IMAGEGRAPHDUAL_H
#define IMAGEGRAPHDUAL_H

#include "ImageGraph.h"
#include "ImageGraphPrimal.h"

class ImageGraphDual : public ImageGraph
{
public:
    ImageGraphDual(const std::string& imageFilename,
                   const std::string& maskFilename);
    virtual ~ImageGraphDual();

    virtual void buildGraph();
    virtual ImageArray runMinCut();

    virtual void setLambda(float lambda);
    virtual void setSigma(float sigma);

    unsigned int numIterations() const;
    void setNumIterations(unsigned int numIterations);

private:
    ImageGraph::ImageArray mergeSolutions(const ImageArray &solutionM,
                                          const ImageArray &solutionN);

private:
    ImageGraphPrimal _subgraphM;
    ImageGraphPrimal _subgraphN;

    unsigned int _splitX;
    std::vector<float> _lagrangians;

    unsigned int _numIterations;
};

#endif // IMAGEGRAPHDUAL_H
