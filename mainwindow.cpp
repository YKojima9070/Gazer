#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QKeyEvent>
#include <QDebug>
#include <QCameraInfo>
#include <QGridLayout>
#include <QIcon>
#include <QStandardItem>
#include <QSize>

#include "opencv2/opencv.hpp"
#include "mainwindow.h"
#include "utilities.h"

//基本の部分テキストには記述されない
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , fileMenu(nullptr)
    , capturer(nullptr)
{
    initUI();
    data_lock = new QMutex();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI(){
    this ->resize(1000, 800);

    fileMenu = menuBar()->addMenu("&File");

    // main area
    QGridLayout *main_layout = new QGridLayout();

#ifdef GAZER_USE_QT_CAMERA
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    camera = new QCamera(cameras[1]);
    viewfinder = new QCameraViewfinder(this);
    QCameraVidwfinderSettings settings;

    settings.setResolution(QSize(800, 600));
    camera->setViewfinder(viewfinder);
    camera->setViewfinderSettings(settings);
    main_layout->addWidget(viewfinder, 0, 0, 12, 1);

#else
    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);

#endif

    QGridLayout *tools_layout =new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);

    monitorCheckBox = new QCheckBox(this);
    monitorCheckBox->setText("Monitor On/OFF");
    tools_layout->addWidget(monitorCheckBox, 0, 0);    

    recordButton = new QPushButton(this);
    recordButton->setText("Record");
    tools_layout->addWidget(recordButton, 0, 1, Qt::AlignHCenter);
    tools_layout->addWidget(new QLabel(this), 0, 2);
    connect(recordButton, SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));

    //list of saved video
    saved_list = new QListView(this);
    saved_list->setViewMode(QListView::IconMode);
    saved_list->setResizeMode(QListView::Adjust);
    saved_list->setSpacing(5);
    saved_list->setWrapping(false);
    list_model = new QStandardItemModel(this);
    saved_list->setModel(list_model);
    main_layout->addWidget(saved_list, 13, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    // setup status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Gazer is Ready");

    createActions();
}

//基本の部分テキストには記述されない
void MainWindow::createActions(){
    cameraInfoAction = new QAction("Camera Information", this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("Open Camera", this);
    fileMenu->addAction(openCameraAction);
    exitAction = new QAction("Exit", this);
    fileMenu->addAction(exitAction);
    calcFPSAction = new QAction("CalcFPS", this);
    fileMenu->addAction(calcFPSAction);


    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(calcFPSAction, SIGNAL(triggered(bool)), this, SLOT(calculateFPS()));
}


void MainWindow::showCameraInfo(){
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");

    foreach(const QCameraInfo &cameraInfo, cameras){
        info += "." + cameraInfo.deviceName() + ":";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras", info);
}

#ifdef GAZER_USE_QT_CAMERA
void MainWindow::openCamera(){
    camera->setCaptureMode(QCamera::CaptureVideo);
    camera->start();
}

#else

void MainWindow::openCamera(){
    if(capturer != nullptr){
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
        disconnect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);
        connect(capturer, &CaptureThread::finished, capturer,&CaptureThread::deleteLater);

    }
    int camID = 0;
    capturer = new CaptureThread(camID, data_lock);
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::fpsChanged, this, &MainWindow::updateFPS);
    connect(capturer, &CaptureThread::videoSaved, this, &MainWindow::appendSavedVideo);

    capturer->start();
    mainStatusLabel->setText(QString("Captureing Camera %1").arg(camID));
}
#endif

void MainWindow::updateFrame(cv::Mat *mat){
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(
                currentFrame.data,
                currentFrame.cols,
                currentFrame.rows,
                currentFrame.step,
                QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);
    imageScene->clear();
    imageView->resetTransform();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());
}

void MainWindow::calculateFPS(){
    if(capturer != nullptr){
        capturer->startCalcFPS();
    }
}

void MainWindow::updateFPS(float fps){
    mainStatusLabel->setText(QString("FPS of current camera is %1").arg(fps));
}

void MainWindow::recordingStartStop(){
    QString text = recordButton->text();
    if(text == "Record" && capturer !=nullptr){
        capturer->setVideoSavingStatus(CaptureThread::STARTING);
        recordButton->setText("Stop Recording");
    }else if(text == "Stop Recording" && capturer !=nullptr){
        capturer->setVideoSavingStatus(CaptureThread::STOPPING);
        recordButton->setText("Record");
    }
}

void MainWindow::appendSavedVideo(QString name){
    QString cover = Utilities::getSavedVideoPath(name, "jpg");
    QStandardItem *item = new QStandardItem();
    list_model->appendRow(item);
    QModelIndex index = list_model->indexFromItem(item);
    list_model->setData(index, QPixmap(cover).scaledToHeight(145), Qt::DecorationRole);
    list_model->setData(index, name, Qt::DisplayRole);
    saved_list->scrollTo(index);
}








