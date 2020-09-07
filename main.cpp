#include "mainwindow.h"
#include <QApplication>

#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

    /*
    VideoCapture cap(0);
    if(!cap.isOpened()){
        return -1;
    }
    while(1){
        Mat frame;
        cap >> frame;
        if(frame .empty()){
            break;
        }
        imshow("frame", frame);

        char c = (char)waitKey(25);
        if(c==27){
            break;
        }

    }
    cap.release();
    destroyAllWindows();

    return 0;
    */

}
