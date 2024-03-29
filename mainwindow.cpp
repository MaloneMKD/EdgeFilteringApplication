#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <qmath.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect buttons to functions
    connect(ui->openImageButton, SIGNAL(clicked(bool)), this, SLOT(openImage()));
    connect(ui->saveAsButton, SIGNAL(clicked(bool)), this, SLOT(saveImage()));
    connect(ui->applyFilterButton, SIGNAL(clicked(bool)), this, SLOT(filter()));
    connect(ui->clearFilterButton, SIGNAL(clicked(bool)), this, SLOT(clearFilter()));
    connect(ui->thresholdSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeThreshold(int)));
    connect(ui->aboutButton, SIGNAL(clicked(bool)), this, SLOT(showAbout()));
    connect(ui->aboutQTButton, SIGNAL(clicked(bool)), this, SLOT(showAboutQT()));

    // Initialize variables
    threshold = 50;
    imageScaled = false;
    filterApplied = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::blur(QImage &img)
{
    int avg = 0;
    for (int y = 0; y < img.height(); ++y)
    {
        for (int x = 0; x < img.width(); ++x)
        {
            avg = getIntensityAverage(x, y, img);
            img.setPixel(x, y, qRgb(avg, avg, avg));
        }
    }
}

void MainWindow::sorbelFilter(QImage &img)
{
    QImage newImg = img;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            int mag = getFilteredPixel(x, y, img);
            newImg.setPixel(x, y, qRgb(mag, mag, mag));
        }
    }
    img = newImg;
}

void MainWindow::openImage()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(), "PNG (*.png)");

    if(filename != "")
    {
        inputImg = QImage(filename);

        QPixmap pic(filename);
        if(pic.width() > 1480 || pic.height() > 720)
        {
            ui->label->setPixmap(pic.scaled(pic.width()/1.5, pic.height()/1.5));
            imageScaled = true;
        }
        else
            ui->label->setPixmap((pic));

        if(imageScaled)
            ui->statusUpdateLabel->setText("Image has been scaled to fit the window. Image will be filtered and saved with original resolution");
        else
           ui->statusUpdateLabel->setText("");
    }
}

void MainWindow::saveImage()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save As", QDir::homePath() + "/untitled.png", "PNG *.png");

    if(filename != "")
    {
        QMessageBox *messageBox = new QMessageBox(this);
        if(filteredImg.save(filename, "PNG"))
        {
            messageBox->setWindowTitle("Success");
            messageBox->setText("The file was saved successfully!\n\nThe file was saved in its original resolution");
            messageBox->setFont(QFont("Segoe UI"));
            messageBox->setIcon(QMessageBox::Information);
            messageBox->show();

            ui->statusUpdateLabel->setText(QString("Saved at path: %1") .arg(filename));
        }
        else
        {
            messageBox->setWindowTitle("Failed");
            messageBox->setText("Error: An error occured while saving the image! Image not saved");
            messageBox->setFont(QFont("Segoe UI"));
            messageBox->setIcon(QMessageBox::Warning);
            messageBox->show();

            ui->statusUpdateLabel->setText("Error Saving file.");
        }
    }
}

void MainWindow::filter()
{
    // Convert image to grayscale
    filteredImg = inputImg.convertedTo(QImage::Format_Grayscale16);

    // Apply blur
    blur(filteredImg);

    // Apply Sorbel Filter
    sorbelFilter(filteredImg);

    // Display image (possibly scaled to width) to screen
    if(filteredImg.width() > 1480 || filteredImg.height() > 720)
        ui->label->setPixmap((QPixmap::fromImage(filteredImg).scaled(filteredImg.width()/1.5, filteredImg.height()/1.5)));
    else
        ui->label->setPixmap((QPixmap::fromImage(filteredImg)));
}

void MainWindow::clearFilter()
{
    filteredImg = inputImg;
    if(filteredImg.width() > 1480 || filteredImg.height() > 720)
        ui->label->setPixmap((QPixmap::fromImage(filteredImg).scaled(filteredImg.width()/1.5, filteredImg.height()/1.5)));
    else
        ui->label->setPixmap((QPixmap::fromImage(filteredImg)));
    ui->statusUpdateLabel->setText("");
}

void MainWindow::changeThreshold(int val)
{
    threshold = val;
}

void MainWindow::showAbout()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setText("Developer: Malone Napier-Jameson\nEmail: MK.Napier-Jameson@outlook.com\n\n"
                  "This application converts a given picture to grayscale, smoothes it out by blurring it and then applies the sorbel filter to it to show its edges."
                  "\n\nThis Program was created using the Qt Framework.");
    mess->setFont(QFont("Segoe UI"));
    mess->setWindowTitle("About");
    mess->show();
}

void MainWindow::showAboutQT()
{
    qApp->aboutQt();
}

int MainWindow::getIntensityAverage(int x, int y, QImage img)
{
    int yOff = 0;
    int sum = 0;
    for(int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            if(x + j >= 0 && y + i >= 0 && x + j < img.width() && y + i < img.height())
                sum += img.pixelColor(x + j, y + i).value();
        }
        yOff += 3;
    }
    return sum/9;
}

int MainWindow::getFilteredPixel(int x, int y, QImage img)
{
    qreal horizontalGradient = 0;
    qreal verticalGradient = 0;
    qreal gMagnitude = 0;
    qreal kernelX[9] = {-1.0, 0.0, 1.0, -2.0, 0.0, 2.0, -1.0, 0.0, 1.0};
    qreal kernelY[9] = {-1.0, -2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0};

    int imageWidth = img.width();
    int imageHeight = img.height();

    int yOff = 1;
    // Get horizontal gradient
    for(int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            // If edge pixel, ignore the missing values
            if(x + j >= 0 && y + i >= 0 && x + j < imageWidth && y + i < imageHeight)
                horizontalGradient += img.pixelColor(x + j, y + i).value() * kernelX[yOff + j];
        }
        yOff += 3;
    }

    // Get vertical gradient
    yOff = 1;
    for(int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            // If edge pixel, ignore the missing values
            if(x + j >= 0 && y + i >= 0 && x + j < imageWidth && y + i < imageHeight)
                verticalGradient += img.pixelColor(x + j, y + i).value() * kernelY[yOff + j];
        }
        yOff += 3;
    }

    // Get magnitude
    gMagnitude = qSqrt(qPow(horizontalGradient, 2)+ qPow(verticalGradient, 2));
    if(gMagnitude < threshold)
    {
        if(ui->invColorCheckBox->isChecked())
            return 0;
        else
            return 255;
    }
    else
    {
        if(ui->invColorCheckBox->isChecked())
            return 255;
        else
            return 0;
    }
}
