/**
 * This is an example project to get started with vigra, lemon,
 * and graph cuts on the way to understand dual decomposition.
 *
 * (c) by Carsten Haubold, HCI, Dec 2013
 */

#include "ImageGraphPrimal.h"
#include "ImageGraphDual.h"
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::cout << "GraphCut example by Carsten Haubold" << std::endl;
    std::cout << "\tPerforms a graph cut on a grayscale image and writes the resulting cut to an image\n" << std::endl;

    // check for command line parameters:
    if(argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " inputImageFilename pixelMaskFilename outputImageFilename" << std::endl;
        return 0;
    }

    //ImageGraphPrimal imageGraph(argv[1], argv[2]);
    ImageGraphDual imageGraph(argv[1], argv[2]);


    MainWindow w;
    w.show();
    w.setImageGraph(&imageGraph);

    return app.exec();
}
