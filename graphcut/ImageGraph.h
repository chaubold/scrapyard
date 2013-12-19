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
#include <lemon/dijkstra.h>
#include <lemon/hartmann_orlin_mmc.h>

typedef lemon::ListGraph Graph;
typedef lemon::ListGraph::EdgeMap<int> EdgeMap;
typedef lemon::ListGraph::Edge Edge;
#define ADD_EDGE(x,y) _graph.addEdge((x), (y))

class ImageGraph {
public:
    typedef vigra::MultiArray<2, vigra::UInt8> ImageArray;
    typedef std::pair<unsigned int, unsigned int> Coordinate;

public:
    ImageGraph(const std::string& filename);
    ~ImageGraph();

    ImageArray runMinCut();

protected:
    void loadImage(const std::string& filename);
    void buildGraph();
    inline void createEdgeToNodeWithIndex(unsigned int x0,
                                          unsigned int y0,
                                          unsigned int x1,
                                          unsigned int y1,
                                          unsigned int width,
                                          Graph::Node& a,
                                          std::vector<Graph::Node>& nodes);

private:
    ImageArray _imageArray;

    Graph _graph;
    EdgeMap _costs;
    Graph::NodeMap<Coordinate> _coordinates;
    Graph::Node _sourceNode;
    Graph::Node _sinkNode;
};


#endif // IMAGEGRAPH_H
