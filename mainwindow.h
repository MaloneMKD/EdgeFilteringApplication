#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Filtering methods
    void blur(QImage &img);
    void sorbelFilter(QImage &img);

public slots:
    void openImage();
    void saveImage();
    void filter();
    void clearFilter();
    void changeThreshold(int val);
    void showAbout();
    void showAboutQT();

private:
    Ui::MainWindow *ui;

    QImage inputImg;
    QImage filteredImg;
    int threshold;
    bool imageScaled;

    // Private functions
    int getFilteredPixel(int x, int y, QImage img);
    int getIntensityAverage(int x, int y, QImage img);
};
#endif // MAINWINDOW_H
