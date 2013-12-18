/**
 * This is an example project to get started with vigra, lemon,
 * and graph cuts on the way to understand dual decomposition.
 *
 * (c) by Carsten Haubold, HCI, Dec 2013
 */

#include <iostream>

// vigra includes
#include <vigra/imageinfo.hxx>

// lemon includes
#include <lemon/list_graph.h>
#include <lemon/preflow.h>

int main(int argc, char *argv[])
{
    std::cout << "GraphCut example by Carsten Haubold\n" << std::endl;

    // check for command line parameters:
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " inputImageFilename" << std::endl;
        return 0;
    }

    // load test image:
    vigra::ImageImportInfo imageInfo(argv[1]);
    std::cout << "Found Image: (" << imageInfo.width() << "x" << imageInfo.height() << ")" << std::endl;

    // compute gradients as edges
    lemon::ListGraph graph;

    lemon::ListGraph::Node n0 = graph.addNode();
    lemon::ListGraph::Node n1 = graph.addNode();
    lemon::ListGraph::Node n2 = graph.addNode();
    lemon::ListGraph::Node n3 = graph.addNode();

    lemon::ListGraph::Edge e0 = graph.addEdge(n0, n1);
    lemon::ListGraph::Edge e1 = graph.addEdge(n1, n2);
    lemon::ListGraph::Edge e2 = graph.addEdge(n2, n3);
    lemon::ListGraph::Edge e3 = graph.addEdge(n0, n2);

    lemon::ListGraph::EdgeMap<int> costs(graph);
    costs[e0] = 3;
    costs[e1] = 2;
    costs[e2] = 5;
    costs[e3] = 2;

    // perform max-flow
    lemon::Preflow< lemon::ListGraph, lemon::ListGraph::EdgeMap<int> > preflow(graph, costs, n0, n3);
    preflow.init();
    preflow.runMinCut();

    lemon::ListGraph::NodeMap<bool> minCut(graph);
    preflow.minCutMap(minCut);

    // output results
    std::cout << "Finished running minCut algorithm." << std::endl;
    if(minCut[n0])
        std::cout << "Node 0 is cut!" << std::endl;

    if(minCut[n1])
        std::cout << "Node 1 is cut!" << std::endl;

    if(minCut[n2])
        std::cout << "Node 2 is cut!" << std::endl;

    if(minCut[n3])
        std::cout << "Node 3 is cut!" << std::endl;
    return 0;
}
