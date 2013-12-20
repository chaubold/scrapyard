#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class ImageGraph;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setImageGraph(ImageGraph *imageGraph);

public slots:
    void setNewLambdaValue(int value);
    void setNewSigmaValue(int value);

private:
    Ui::MainWindow *_ui;
    ImageGraph* _imageGraph;
    QVector<QRgb> _colorTable;

    void processImage();
};

#endif // MAINWINDOW_H
