#ifndef IMAGEGRAPH_H
#define IMAGEGRAPH_H

#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

// vigra includes
#include <vigra/impex.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/imageinfo.hxx>

// lemon includes
#include <lemon/list_graph.h>
#include <lemon/preflow.h>

// own includes
#include "PixelMask.h"

typedef lemon::ListGraph Graph;
typedef lemon::ListGraph::EdgeMap<float> EdgeMap;
typedef lemon::ListGraph::Edge Edge;
#define ADD_EDGE(x,y) _graph.addEdge((x), (y))

#define SPLIT_NOT_SET 0xFFFFFFFF

class ImageGraph {
public:
    typedef vigra::MultiArray<2, vigra::UInt8> ImageArray;
    typedef std::pair<unsigned int, unsigned int> Coordinate;

public:
    ImageGraph(const std::string &imageFilename, const std::string &maskFilename);
    virtual ~ImageGraph();

    virtual void buildGraph() = 0;
    virtual ImageArray runMinCut() = 0;

    float lambda() const;
    virtual void setLambda(float lambda);

    float sigma() const;
    virtual void setSigma(float sigma);

private:
    void loadImage(const std::string& filename);

protected:
    ImageArray _imageArray;
    PixelMask _pixelMask;

    float _maxBoundaryPenalty;
    float _lambda;
    float _sigma;

    Graph _graph;
    EdgeMap _costs;
    Graph::NodeMap<Coordinate> _coordinates;
    Graph::Node _sourceNode;
    Graph::Node _sinkNode;
};


#endif // IMAGEGRAPH_H
