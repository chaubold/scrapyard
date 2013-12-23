#include "ImageGraphDual.h"
#include <QtConcurrentRun>
#include <chrono>

ImageGraphDual::ImageGraphDual(const std::string &imageFilename,
                               const std::string &maskFilename):
    ImageGraph(imageFilename, maskFilename),
    _subgraphM(imageFilename, maskFilename),
    _subgraphN(imageFilename, maskFilename),
    _splitX(_imageArray.shape(0)/2)
{
    // set up both subgraphs with overlap
    _subgraphM.setLoggingEnabled(false);
    _subgraphM.setRange(0, 0, _splitX + 1, _imageArray.shape(1));

    _subgraphN.setLoggingEnabled(false);
    _subgraphN.setRange(_splitX, 0, _imageArray.shape(0), _imageArray.shape(1));
}

ImageGraphDual::~ImageGraphDual() {}

void ImageGraphDual::buildGraph()
{
    auto start = std::chrono::high_resolution_clock::now();

    // build subgraphs in parallel
    QFuture<void> fM = QtConcurrent::run(&_subgraphM, &ImageGraphPrimal::buildGraph);
    QFuture<void> fN = QtConcurrent::run(&_subgraphN, &ImageGraphPrimal::buildGraph);

    fM.waitForFinished();
    fN.waitForFinished();

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "Building Graphs took: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;
}

ImageGraph::ImageArray ImageGraphDual::runMinCut()
{
    auto start = std::chrono::high_resolution_clock::now();

    // loop for K iterations

    // find solution of subproblems in parallel
    QFuture<ImageGraph::ImageArray> fM = QtConcurrent::run(&_subgraphM, &ImageGraphPrimal::runMinCut);
    QFuture<ImageGraph::ImageArray> fN = QtConcurrent::run(&_subgraphN, &ImageGraphPrimal::runMinCut);

    fM.waitForFinished();
    fN.waitForFinished();

    // check how much the results in the overlap differ

    // return solution if subproblems agree
    // return mergeSolutions(fM.result(), fN.result());

    // update lagrangians

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "Finding MinCut took: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;

    return mergeSolutions(fM.result(), fN.result());
}

void ImageGraphDual::setLambda(float lambda)
{
    ImageGraph::setLambda(lambda);
    _subgraphM.setLambda(lambda);
    _subgraphN.setLambda(lambda);
}

void ImageGraphDual::setSigma(float sigma)
{
    ImageGraph::setSigma(sigma);
    _subgraphM.setSigma(sigma);
    _subgraphN.setSigma(sigma);
}

ImageGraph::ImageArray ImageGraphDual::mergeSolutions(
        const ImageGraph::ImageArray &solutionM,
        const ImageGraph::ImageArray &solutionN)
{
    ImageGraph::ImageArray result(solutionM);

    return result;
}
