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

class ImageGraph {
public:
    typedef vigra::MultiArray<2, vigra::UInt8> ImageArray;
    typedef std::pair<unsigned int, unsigned int> Coordinate;

public:
    ImageGraph(const std::string& imageFilename, const std::string& maskFilename);
    ~ImageGraph();

    void buildGraph();
    ImageArray runMinCut();

    float lambda() const;
    void setLambda(float lambda);

    float sigma() const;
    void setSigma(float sigma);

private:
    void loadImage(const std::string& filename);

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
    void addRegionEdgesAndPenalties(unsigned int height, unsigned int width, std::vector<Graph::Node> &nodes);

private:
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
