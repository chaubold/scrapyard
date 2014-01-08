#include "ImageGraphDual.h"
#include <QtConcurrentRun>
#include <chrono>

ImageGraphDual::ImageGraphDual(const std::string &imageFilename,
                               const std::string &maskFilename):
    ImageGraph(imageFilename, maskFilename),
    _subgraphM(imageFilename, maskFilename),
    _subgraphN(imageFilename, maskFilename),
    _splitX(_imageArray.shape(0)/2),
    _lagrangians(_imageArray.shape(1), 0),
    _numIterations(1)
{
    // set up both subgraphs with overlap
    _subgraphM.setLoggingEnabled(false);
    _subgraphM.setRange(0, 0, _splitX + 1, _imageArray.shape(1));
    _subgraphM.setSplitX(_splitX);

    _subgraphN.setLoggingEnabled(false);
    _subgraphN.setRange(_splitX, 0, _imageArray.shape(0), _imageArray.shape(1));
    _subgraphN.setSplitX(_splitX);
}

ImageGraphDual::~ImageGraphDual() {}

void ImageGraphDual::buildGraph()
{
    auto start = std::chrono::high_resolution_clock::now();

    _subgraphM.setLagrangians(_lagrangians);

    std::vector<float> negativeLagrangians(_lagrangians);
    std::transform(_lagrangians.begin(), _lagrangians.end(), negativeLagrangians.begin(), [](float x){return -x;});
    _subgraphN.setLagrangians(negativeLagrangians);

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
    auto sum = 0;
    QFuture<ImageGraph::ImageArray> fM;
    QFuture<ImageGraph::ImageArray> fN;

    // loop for K iterations
    for(auto iteration = 0; iteration < _numIterations; iteration++)
    {
        if(iteration > 0)
        {
            buildGraph();
        }

        // find solution of subproblems in parallel
        fM = QtConcurrent::run(&_subgraphM, &ImageGraphPrimal::runMinCut);
        fN = QtConcurrent::run(&_subgraphN, &ImageGraphPrimal::runMinCut);

        fM.waitForFinished();
        fN.waitForFinished();

        // check how much the results in the overlap differ
        sum = 0;
        for(auto i = 0; i < _lagrangians.size(); i++)
        {
            bool nodeInSourceSetForM = _subgraphM.isNodeInSourceSubset(_splitX, i);
            bool nodeInSourceSetForN = _subgraphN.isNodeInSourceSubset(_splitX, i);

            if(nodeInSourceSetForM != nodeInSourceSetForN)
            {
                std::cout << "Pixel (" << _splitX << ", " << i << ") disagrees!" << std::endl;
                sum++;
            }
        }

        if(sum == 0)
        {
            break;
        }

        // update lagrangians
        std::cout << "\nIteration " << iteration << ": There were " << sum << " disagreeing pixels\n" << std::endl;

        for(auto i = 0; i < _lagrangians.size(); i++)
        {
            int nodeInSourceSetForM = _subgraphM.isNodeInSourceSubset(_splitX, i)?1:0;
            int nodeInSourceSetForN = _subgraphN.isNodeInSourceSubset(_splitX, i)?1:0;

            if(nodeInSourceSetForM != nodeInSourceSetForN)
            {
                // stick to a stepsize of 1 for now
                _lagrangians[i] -= 100 * (nodeInSourceSetForM - nodeInSourceSetForN);
            }
        }

        std::cout << "\tLagrangian: min=" << *(std::min_element(_lagrangians.begin(), _lagrangians.end()))
                     << " max=" << *(std::max_element(_lagrangians.begin(), _lagrangians.end())) << std::endl;
    }
    // loop end

    std::cout << "\nEnd: There were " << sum << " disagreeing pixels\n" << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "Solving with Dual Decomposition took: " << 0.001f * elapsed_milliseconds << " secs" << std::endl;

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

    try
    {
        result.subarray(vigra::Shape2(_splitX, 0), _imageArray.shape()) =
                solutionN.subarray(vigra::Shape2(_splitX, 0), _imageArray.shape());
    }
    catch(std::exception e)
    {
        std::cout << "Error when copying: " << e.what() << std::endl;
    }

    return result;
}
unsigned int ImageGraphDual::numIterations() const
{
    return _numIterations;
}

void ImageGraphDual::setNumIterations(unsigned int numIterations)
{
    _numIterations = numIterations;
}

