#include "ImageGraph.h"

#define MAX_COST 65025

ImageGraph::ImageGraph(const std::string &imageFilename, const std::string &maskFilename):
    _imageArray(),
    _graph(),
    _costs(_graph),
    _coordinates(_graph)
{
    loadImage(imageFilename);
    _pixelMask = PixelMask(maskFilename, &_imageArray);
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

    _costs[e] = 65025 - gradientMagnitude;
}

void ImageGraph::insertEdgeToSource(unsigned int x, unsigned int y, Graph::Node& a, vigra::UInt8 pixelValue)
{
    Edge e = ADD_EDGE(a, _sourceNode);

    if(_pixelMask.pixelIsForeground(x, y))
        _costs[e] = MAX_COST;
    else if(_pixelMask.pixelIsBackground(x, y))
        _costs[e] = 0;
    else
        _costs[e] = _pixelMask.probabilityOfBeingForeground(pixelValue);
}

void ImageGraph::insertEdgeToSink(unsigned int x, unsigned int y, Graph::Node& a, vigra::UInt8 pixelValue)
{
    Edge e = ADD_EDGE(a, _sinkNode);

    if(_pixelMask.pixelIsBackground(x, y))
        _costs[e] = MAX_COST;
    else if(_pixelMask.pixelIsForeground(x, y))
        _costs[e] = 0;
    else
        _costs[e] = _pixelMask.probabilityOfBeingBackground(pixelValue);
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
        }
    }

    // insert sink and source
    _sinkNode = _graph.addNode();
    _sourceNode = _graph.addNode();

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

            // add edges to source and sink, weighted by the pixel color
            vigra::UInt8 pixelValue = _imageArray(x, y);
            insertEdgeToSource(x, y, a, pixelValue);
            insertEdgeToSink(x, y, a, pixelValue);
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

ImageGraph::ImageArray ImageGraph::runMinCut()
{
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

    std::cout << "#### Nodes on the cut: " << numNodesOnCut << " (" << 100.0f*(float)numNodesOnCut / (_imageArray.shape(0) * _imageArray.shape(1)) << "%)" << std::endl;

    return cutImage;
}
