/**
 * This is an example project to get started with vigra, lemon,
 * and graph cuts on the way to understand dual decomposition.
 *
 * (c) by Carsten Haubold, HCI, Dec 2013
 */

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

//typedef lemon::ListDigraph Graph;
//typedef lemon::ListDigraph::ArcMap<int> EdgeMap;
//typedef lemon::ListDigraph::Arc Edge;
//#define ADD_EDGE(x,y) _graph.addArc((x), (y))

class ImageGraph {
public:
    typedef vigra::MultiArray<2, vigra::UInt8> ImageArray;
    typedef std::pair<unsigned int, unsigned int> Coordinate;

public:
    ImageGraph(const std::string& filename);
    ~ImageGraph();

    ImageArray runMinCut(Coordinate source, Coordinate sink);
    ImageArray runDijkstra(Coordinate source, Coordinate sink);
    ImageArray runMinMeanCycle(Coordinate source, Coordinate sink);

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
};

ImageGraph::ImageGraph(const std::string &filename):
    _imageArray(),
    _graph(),
    _costs(_graph),
    _coordinates(_graph)
{
    loadImage(filename);
    buildGraph();
}

ImageGraph::~ImageGraph() {}

void ImageGraph::loadImage(const std::string& filename)
{
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
}

void ImageGraph::createEdgeToNodeWithIndex(unsigned int x0,
                                           unsigned int y0,
                                           unsigned int x1,
                                           unsigned int y1,
                                           unsigned int width,
                                           Graph::Node& a,
                                           std::vector<Graph::Node>& nodes)
{
    Graph::Node& b = nodes[y0 * width + x0];
    Edge e = ADD_EDGE(a, b);

    // compute gradient magnitude
    int gradientMagnitude = _imageArray(x0, y0) - _imageArray(x1, y1);
    gradientMagnitude *= gradientMagnitude;

    //std::cout << "Setting edge weight: " << gradientMagnitude << std::endl;
    _costs[e] = 10000 - gradientMagnitude;
}

void ImageGraph::buildGraph()
{
    unsigned int width = _imageArray.shape(0);
    unsigned int height = _imageArray.shape(1);


    // create nodes
    std::cout << "Generating graph nodes..." << std::endl;
    std::vector<Graph::Node> nodes(width*height);
    std::generate(nodes.begin(), nodes.end(), [&]() { return _graph.addNode(); });

    // fill the coordinate map
    std::cout << "Filling coordinate map..." << std::endl;
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            _coordinates[nodes[y * width + x]] = std::make_pair(x, y);
            //std::cout << "Node (" << x << ", " << y << "): ID=" << _graph.id(nodes[y * width + x]) << std::endl;
        }
    }

    // add edges and their weights
    std::cout << "Adding Edges and their weights..." << std::endl;
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            Graph::Node& a = nodes[y * width + x];

            // for all but the last row insert edge to the next node in y
            if(y < height - 1)
            {
                createEdgeToNodeWithIndex(x, y, x, y+1, width, a, nodes);
            }

            // for all but the last column insert edge to the next node in x
            if(x < width- 1)
            {
                createEdgeToNodeWithIndex(x, y, x+1, y, width, a, nodes);
            }
        }
    }

    // Some DEBUG information
    int minCost = 10000000;
    int maxCost = 0;
    for(Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e)
    {
        minCost = std::min(minCost, _costs[e]);
        maxCost = std::max(maxCost, _costs[e]);
    }

    std::cout << "MinCost: " << minCost << "\nmaxCost: " << maxCost << std::endl;
}

ImageGraph::ImageArray ImageGraph::runMinMeanCycle(Coordinate source, Coordinate sink)
{
    // find source and sink nodes
    Graph::Node sourceNode = _graph.nodeFromId(source.second * _imageArray.shape(0) + source.first);
    Graph::Node sinkNode = _graph.nodeFromId(sink.second * _imageArray.shape(0) + sink.first);

    // perform min-cut / max-flow
    std::cout << "Running Min Mean Cycle..." << std::endl;
    lemon::HartmannOrlinMmc< Graph, EdgeMap > minMeanCycle(_graph, _costs);
    if(!minMeanCycle.run())
    {
        std::cout << "Couldn't find min mean cycle" << std::endl;
        exit(0);
    }

    // extract nodes on the cut
    std::cout << "Extracting results..." << std::endl;
    std::cout << "Cycle Size: " << minMeanCycle.cycleSize() << std::endl;
    std::cout << "Cycle Mean: " << minMeanCycle.cycleMean() << std::endl;

    // create vigra image of the cut
    ImageArray cutImage(_imageArray.shape());
    cutImage = 0;

    unsigned int numNodesOnCycle = 0;

//    for (Graph::NodeIt n(_graph); n != lemon::INVALID; ++n)
//    {
//        if(preflow.minCut(n))
//        {
//            Coordinate c = _coordinates[n];
//            cutImage(c.first, c.second) = 255;
//            numNodesOnCycle++;
//        }
//    }

    std::cout << "#### Nodes on the cut: " << numNodesOnCycle << " (" << 100.0f*(float)numNodesOnCycle / (_imageArray.shape(0) * _imageArray.shape(1)) << "%)" << std::endl;

    return cutImage;
}

ImageGraph::ImageArray ImageGraph::runMinCut(Coordinate source, Coordinate sink)
{
    // find source and sink nodes
    Graph::Node sourceNode = _graph.nodeFromId(source.second * _imageArray.shape(0) + source.first);
    Graph::Node sinkNode = _graph.nodeFromId(sink.second * _imageArray.shape(0) + sink.first);

    // perform min-cut / max-flow
    std::cout << "Running Min-Cut..." << std::endl;
    lemon::Preflow< Graph, EdgeMap > preflow(_graph, _costs, sourceNode, sinkNode);
    preflow.init();
    //preflow.runMinCut();
    preflow.run();

    // extract nodes on the cut
    std::cout << "Extracting results..." << std::endl;
    std::cout << "Maximum Flow: " << preflow.flowValue() << std::endl;

    // create vigra image of the cut
    ImageArray cutImage(_imageArray.shape());
    cutImage = 0;

    unsigned int numNodesOnCut = 0;

    for (Graph::NodeIt n(_graph); n != lemon::INVALID; ++n)
    {
        if(preflow.minCut(n))
        {
            Coordinate c = _coordinates[n];
            cutImage(c.first, c.second) = 255;
            numNodesOnCut++;
        }
    }

    std::cout << "#### Nodes on the cut: " << numNodesOnCut << " (" << 100.0f*(float)numNodesOnCut / (_imageArray.shape(0) * _imageArray.shape(1)) << "%)" << std::endl;

    return cutImage;
}

ImageGraph::ImageArray ImageGraph::runDijkstra(Coordinate source, Coordinate sink)
{
    // find source and sink nodes
    Graph::Node sourceNode = _graph.nodeFromId(source.second * _imageArray.shape(0) + source.first);
    Graph::Node sinkNode = _graph.nodeFromId(sink.second * _imageArray.shape(0) + sink.first);

    std::cout << "Running Dijkstra" << std::endl;
    lemon::Dijkstra< Graph, EdgeMap > shortestPath(_graph, _costs);
    lemon::Dijkstra< Graph, EdgeMap >::DistMap distances(_graph);
    shortestPath.distMap(distances);
    shortestPath.init();
//    shortestPath.addSource(sourceNode);
//    shortestPath.start();
    shortestPath.run(Graph::NodeIt(_graph));

//    if(!shortestPath.run(sourceNode, sinkNode))
//    {
//        std::cout << "Cannot reach target from source" << std::endl;
//        exit(0);
//    }

//    lemon::Dijkstra< Graph, EdgeMap >::Path path = shortestPath.path(sinkNode);
//    std::cout << "Length of shortest path: " << path.length() << std::endl;

    // extract nodes on the cut
    std::cout << "Extracting results..." << std::endl;

    // create vigra image of the cut
    ImageArray cutImage(_imageArray.shape());
    cutImage = 0;

    unsigned int minDistance = 0xFFFFFFFF;
    unsigned int maxDistance = 0;
    unsigned int numProcessed = 0;

    for (Graph::NodeIt n(_graph); n != lemon::INVALID; ++n)
    {
        if(shortestPath.processed(n))
        {
            Coordinate c = _coordinates[n];
            cutImage(c.first, c.second) = shortestPath.dist(n) / 1000;

            minDistance = std::min(minDistance, (unsigned int)shortestPath.dist(n));
            maxDistance = std::max(maxDistance, (unsigned int)shortestPath.dist(n));
            numProcessed++;
        }
    }

    std::cout << "Min Distance = " << minDistance << std::endl;
    std::cout << "Max Distance = " << maxDistance << std::endl;
    std::cout << "Number of nodes processed: " << numProcessed << std::endl;

    return cutImage;
}


int main(int argc, char *argv[])
{
    std::cout << "GraphCut example by Carsten Haubold" << std::endl;
    std::cout << "\tPerforms a graph cut on a grayscale image and writes the resulting cut to an image\n" << std::endl;

    // check for command line parameters:
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " inputImageFilename outputImageFilename" << std::endl;
        return 0;
    }

    ImageGraph imageGraph(argv[1]);
    ImageGraph::ImageArray result = imageGraph.runMinCut(std::make_pair(30,30), std::make_pair(130, 100));
    //ImageGraph::ImageArray result = imageGraph.runDijkstra(std::make_pair(30,30), std::make_pair(130, 100));
    //ImageGraph::ImageArray result = imageGraph.runMinMeanCycle(std::make_pair(30,30), std::make_pair(130, 100));

    std::cout << "Saving result to image: " << argv[1] << std::endl;
    vigra::exportImage(result, argv[2]);

    return 0;
}
