/**
 * This is an example project to get started with vigra, lemon,
 * and graph cuts on the way to understand dual decomposition.
 *
 * (c) by Carsten Haubold, HCI, Dec 2013
 */

#include "ImageGraph.h"

int main(int argc, char *argv[])
{
    std::cout << "GraphCut example by Carsten Haubold" << std::endl;
    std::cout << "\tPerforms a graph cut on a grayscale image and writes the resulting cut to an image\n" << std::endl;

    // check for command line parameters:
    if(argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " inputImageFilename pixelMaskFilename outputImageFilename" << std::endl;
        return 0;
    }

    ImageGraph imageGraph(argv[1], argv[2]);
    ImageGraph::ImageArray result = imageGraph.runMinCut();

    std::cout << "Saving result to image: " << argv[3] << std::endl;
    vigra::exportImage(result, argv[3]);

    return 0;
}
