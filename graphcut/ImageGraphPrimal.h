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

    // methods for working as a subproblem to Dual
    void setRange(unsigned int minX, unsigned int minY, unsigned int maxX, unsigned int maxY);
    void setLoggingEnabled(bool enableLog);
    bool isNodeInSourceSubset(unsigned int globalX, unsigned int globalY);

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

private:
    unsigned int _minX;
    unsigned int _minY;
    unsigned int _maxX;
    unsigned int _maxY;

    lemon::Preflow< Graph, EdgeMap > *_preflow;
    std::vector<Graph::Node> _nodes;
    bool _loggingEnabled;
};


#endif // IMAGEGRAPHPRIMAL_H
