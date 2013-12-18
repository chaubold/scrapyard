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

class ImageGraph {
public:
    typedef vigra::MultiArray<2, vigra::UInt8> ImageArray;
    typedef std::pair<unsigned int, unsigned int> Coordinate;

public:
    ImageGraph(const std::string& filename);
    ~ImageGraph();

    ImageArray runMinCut(Coordinate source, Coordinate sink);

protected:
    void loadImage(const std::string& filename);
    void buildGraph();
    inline void createEdgeToNodeWithIndex(unsigned int x0,
                                          unsigned int y0,
                                          unsigned int x1,
                                          unsigned int y1,
                                          unsigned int width,
                                          lemon::ListGraph::Node& a,
                                          std::vector<lemon::ListGraph::Node>& nodes);

private:
    ImageArray _imageArray;

    lemon::ListGraph _graph;
    lemon::ListGraph::EdgeMap<int> _costs;
    lemon::ListGraph::NodeMap<Coordinate> _coordinates;
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
                                           lemon::ListGraph::Node& a,
                                           std::vector<lemon::ListGraph::Node>& nodes)
{
    lemon::ListGraph::Node& b = nodes[y0 * width + x0];
    lemon::ListGraph::Edge e = _graph.addEdge(a, b);

    // compute gradient magnitude
    int gradientMagnitude = _imageArray(x0, y0) - _imageArray(x1, y1);
    gradientMagnitude *= gradientMagnitude;

    _costs[e] = gradientMagnitude;
}

void ImageGraph::buildGraph()
{
    unsigned int width = _imageArray.shape(0);
    unsigned int height = _imageArray.shape(1);


    // create nodes
    std::cout << "Generating graph nodes..." << std::endl;
    std::vector<lemon::ListGraph::Node> nodes(width*height);
    std::generate(nodes.begin(), nodes.end(), [&]() { return _graph.addNode(); });


    // fill the coordinate map
    std::cout << "Filling coordinate map..." << std::endl;
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            _coordinates[nodes[y * height + x]] = std::make_pair(x, y);
        }
    }

    // add edges and their weights
    std::cout << "Adding Edges and their weights..." << std::endl;
    for(unsigned int y = 0; y < height; y++)
    {
        for(unsigned int x = 0; x < width; x++)
        {
            lemon::ListGraph::Node& a = nodes[y * width + x];

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
}

ImageGraph::ImageArray ImageGraph::runMinCut(Coordinate source, Coordinate sink)
{
    // find source and sink nodes
    lemon::ListGraph::Node sourceNode = _graph.nodeFromId(source.second * _imageArray.shape(1) + source.first);
    lemon::ListGraph::Node sinkNode = _graph.nodeFromId(sink.second * _imageArray.shape(1) + sink.first);

    // perform min-cut / max-flow
    std::cout << "Running Min-Cut..." << std::endl;
    lemon::Preflow< lemon::ListGraph, lemon::ListGraph::EdgeMap<int> > preflow(_graph, _costs, sourceNode, sinkNode);
    preflow.init();
    preflow.runMinCut();

    // extract nodes on the cut
    std::cout << "Extracting results..." << std::endl;
    lemon::ListGraph::NodeMap<bool> minCut(_graph);
    preflow.minCutMap(minCut);

    // create vigra image of the cut
    ImageArray cutImage(_imageArray.shape());
    cutImage = 0;
    for(lemon::ListGraph::NodeMap<bool>::MapIt it(minCut); it != lemon::INVALID; ++it)
    {
        lemon::ListGraph::Node current = it;
        Coordinate c = _coordinates[current];
        cutImage(c.first, c.second) = 255;
    }

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

    std::cout << "Saving result to image: " << argv[1] << std::endl;
    vigra::exportImage(result, argv[2]);

    return 0;
}
