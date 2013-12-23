#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ImageGraph.h"
#include <assert.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _imageGraph(NULL),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    QObject::connect(_ui->lambdaSlider, SIGNAL(valueChanged(int)), this, SLOT(setNewLambdaValue(int)));
    QObject::connect(_ui->sigmaSlider, SIGNAL(valueChanged(int)), this, SLOT(setNewSigmaValue(int)));

    for(int i = 0; i < 256; i++) _colorTable.push_back(qRgb(i,i,i));
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::setNewLambdaValue(int value)
{
    float newValue = (float)value; // / _ui->lambdaSlider->maximum();
    _imageGraph->setLambda(newValue);
    _imageGraph->buildGraph();
    processImage();
}

void MainWindow::setNewSigmaValue(int value)
{
    float newValue = (float)value; // / _ui->lambdaSlider->maximum();
    _imageGraph->setSigma(newValue);
    _imageGraph->buildGraph();
    processImage();
}

void MainWindow::setImageGraph(ImageGraph *imageGraph)
{
    _imageGraph = imageGraph;
    _imageGraph->setLambda(_ui->lambdaSlider->value());
    _imageGraph->setSigma(_ui->sigmaSlider->value());
    _imageGraph->buildGraph();
    processImage();
}

void MainWindow::processImage()
{
    assert(_imageGraph);

    ImageGraph::ImageArray result = _imageGraph->runMinCut();
    QImage image = QImage(result.data(), result.shape(0), result.shape(1), QImage::Format_Indexed8);
    image.setColorTable(_colorTable);

    QPixmap pixmap;
    pixmap.convertFromImage(image);
    _ui->imageLabel->setPixmap(pixmap);
}
