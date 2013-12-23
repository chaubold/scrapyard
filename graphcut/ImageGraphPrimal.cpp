#include "ImageGraphPrimal.h"
#include <chrono>

ImageGraphPrimal::ImageGraphPrimal(const std::string &imageFilename, const std::string &maskFilename):
    ImageGraph(imageFilename, maskFilename)
{
}

ImageGraphPrimal::~ImageGraphPrimal() {}

void ImageGraphPrimal::createEdgeToNodeWithIndex(unsigned int x0,
                                           unsigned int y0,
                                           unsigned int x1,
                                           unsigned int y1,
                                           unsigned int width,
                                           Graph::Node& a,
                                           std::vector<Graph::Node>& nodes)
{
    Graph::Node& b = nodes[y1 * width + x1];
    Edge e = ADD_EDGE(a, b);

    // compute gradient magnitude
    float gradientMagnitude = (float)(_imageArray(x0, y0) - _imageArray(x1, y1)); // / 255.0f;
    gradientMagnitude *= gradientMagnitude;

    float distance = sqrtf((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
    float boundaryPenalty = 100.0f * expf(-gradientMagnitude / (2.0f * powf(_sigma,2.0f))) / distance;
    _maxBoundaryPenalty = std::max(_maxBoundaryPenalty, boundaryPenalty);

    _costs[e] = boundaryPenalty;
}

void ImageGraphPrimal::insertEdgeToSource(unsigned int x, unsigned int y, Graph::Node& a, vigra::UInt8 pixelValue)
{
    Edge e = ADD_EDGE(a, _sourceNode);

    if(_pixelMask.pixelIsForeground(x, y))
        _costs[e] = _maxBoundaryPenalty;
    else if(_pixelMask.pixelIsBackground(x, y))
        _costs[e] = 0;
    else
        _costs[e] = _lambda * _pixelMask.backgroundRegionPenalty(pixelValue);
}

void ImageGraphPrimal::insertEdgeToSink(unsigned int x, unsigned int y, Graph::Node& a, vigra::UInt8 pixelValue)
{
    Edge e = ADD_EDGE(a, _sinkNode);

    if(_pixelMask.pixelIsBackground(x, y))
        _costs[e] = _maxBoundaryPenalty;
    else if(_pixelMask.pixelIsForeground(x, y))
        _costs[e] = 0;
    else
        _costs[e] = _lambda * _pixelMask.foregroundRegionPenalty(pixelValue);
}

void ImageGraphPrimal::addBoundaryEdgesAndPenalties(
        unsigned int width,
        unsigned int height,
        std::vector<Graph::Node>& nodes)
{
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
            if(x < width - 1)
            {
                createEdgeToNodeWithIndex(x, y, x+1, y, width, a, nodes);
            }

            // diagonal connection as well
            if(x < width - 1 && y < height - 1)
            {
                createEdgeToNodeWithIndex(x, y, x+1, y+1, width, a, nodes);
            }

            if(x < width - 1 && y > 1)
            {
                createEdgeToNodeWithIndex(x, y, x+1, y-1, width, a, nodes);
            }
        }
    }
}

void ImageGraphPrimal::addRegionEdgesAndPenalties(
        unsigned int width,
        unsigned int height,
        std::vector<Graph::Node>& nodes)
{
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            Graph::Node& a = nodes[y * width + x];
            // add edges to source and sink, weighted by the pixel color
            vigra::UInt8 pixelValue = _imageArray(x, y);
            insertEdgeToSource(x, y, a, pixelValue);
            insertEdgeToSink(x, y, a, pixelValue);
        }
    }
}
float ImageGraphPrimal::sigma() const
{
    return _sigma;
}

void ImageGraphPrimal::setSigma(float sigma)
{
    _sigma = sigma;
}

float ImageGraphPrimal::lambda() const
{
    return _lambda;
}

void ImageGraphPrimal::setLambda(float lambda)
{
    _lambda = lambda;
}


void ImageGraphPrimal::buildGraph()
{
    auto start = std::chrono::high_resolution_clock::now();
    unsigned int width = _imageArray.shape(0);
    unsigned int height = _imageArray.shape(1);

    _graph.clear();
    _maxBoundaryPenalty = 0.0f;

    // create nodes
    std::cout << "Generating graph nodes with lambda=" << _lambda << " and sigma=" << _sigma << "..." << std::endl;
    std::vector<Graph::Node> nodes(width*height);
    std::generate(nodes.begin(), nodes.end(), [&]() { return _graph.addNode(); });

    // fill the coordinate map
    std::cout << "Filling coordinate map..." << std::endl;
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            _coordinates[nodes[y * width + x]] = std::make_pair(x, y);
        }
    }

    // insert sink and source
    _sinkNode = _graph.addNode();
    _sourceNode = _graph.addNode();

    // add edges and their weights
    addBoundaryEdgesAndPenalties(width, height, nodes);

    _maxBoundaryPenalty += 1.0f;

    addRegionEdgesAndPenalties(width, height, nodes);

    // Some DEBUG information
    float minCost = MAXFLOAT;
    float maxCost = 0;
    for(Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e)
    {
        minCost = std::min(minCost, _costs[e]);
        maxCost = std::max(maxCost, _costs[e]);
    }

    std::cout << "MinCost: " << minCost <<
                 "\nmaxCost: " << maxCost <<
                 "\nmaxBoundaryPenalty: " << _maxBoundaryPenalty <<
                 "\nMaxGraphId: " << _graph.maxNodeId() <<
                 "\nMaxEdgeId: " << _graph.maxEdgeId() << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "== Elapsed time: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;
}

ImageGraph::ImageArray ImageGraphPrimal::runMinCut()
{
    auto start = std::chrono::high_resolution_clock::now();
    // perform min-cut / max-flow
    std::cout << "Running Min-Cut..." << std::endl;
    lemon::Preflow< Graph, EdgeMap > preflow(_graph, _costs, _sourceNode, _sinkNode);
    preflow.init();
    preflow.runMinCut();

    // extract nodes on the cut
    std::cout << "Extracting results..." << std::endl;

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

    std::cout << "#### Nodes on the cut: " << numNodesOnCut << " (" <<
                 100.0f*(float)numNodesOnCut / (_imageArray.shape(0) * _imageArray.shape(1)) << "%)" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "== Elapsed time: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;
    std::cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n" << std::endl;
    return cutImage;
}
