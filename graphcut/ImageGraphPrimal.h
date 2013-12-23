#ifndef IMAGEGRAPHPRIMAL_H
#define IMAGEGRAPHPRIMAL_H

#include "ImageGraph.h"

class ImageGraphPrimal : public ImageGraph {
public:
    ImageGraphPrimal(const std::string& imageFilename, const std::string& maskFilename);
    virtual ~ImageGraphPrimal();

    virtual void buildGraph();
    virtual ImageArray runMinCut();

    virtual float lambda() const;
    virtual void setLambda(float lambda);

    virtual float sigma() const;
    virtual void setSigma(float sigma);

private:
    inline void createEdgeToNodeWithIndex(unsigned int x0,
                                          unsigned int y0,
                                          unsigned int x1,
                                          unsigned int y1,
                                          unsigned int width,
                                          Graph::Node& a,
                                          std::vector<Graph::Node>& nodes);

    inline void insertEdgeToSink(unsigned int x, unsigned int y, Graph::Node &a, vigra::UInt8 pixelValue);
    inline void insertEdgeToSource(unsigned int x, unsigned int y, Graph::Node &a, vigra::UInt8 pixelValue);

    void addBoundaryEdgesAndPenalties(unsigned int width, unsigned int height, std::vector<Graph::Node> &nodes);
    void addRegionEdgesAndPenalties(unsigned int width, unsigned int height, std::vector<Graph::Node> &nodes);
};


#endif // IMAGEGRAPHPRIMAL_H
