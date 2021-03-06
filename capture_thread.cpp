#include <QMutex>
#include <QTime>
#include <QElapsedTimer>
#include <iostream>

#include "capture_thread.h"
#include "utilities.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    fps_calculating = false;
    fps = 0.0;
    frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    fps_calculating = false;
    fps = 0.0;
    frame_width = frame_height = 0;
    video_saving_status = STOPPED;
    saved_video_name = "";
    video_writer = nullptr;
}

CaptureThread::~CaptureThread()
{
}

void CaptureThread::run()
{
    running = true;
    cv::VideoCapture cap(cameraID);
    //cv::VideoCapture cap(1);

    cv::Mat tmp_frame;

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, FIXED_FRAME);


    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    cap_count = 0;

    while(running){
        cap >> tmp_frame;
        if (tmp_frame.empty()){
            break;
        }

        if(video_saving_status == STARTING){
            startSavingVideo(tmp_frame);
        }
        if(video_saving_status == STARTED){
            if(cap_count >= FIXED_FRAME*60*CAP_MIN){
                stopSavingVideo();
                startSavingVideo(tmp_frame);
                cap_count = 0;
            }
            video_writer->write(tmp_frame);
        }
        if(video_saving_status == STOPPING){
            stopSavingVideo();
        }

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;

        cap_count ++;

        data_lock->unlock();
        emit frameCaptured(&frame);
        if(fps_calculating){
            calculateFPS(cap);
        }
    }
    cap.release();
    running = false;
}

void CaptureThread::calculateFPS(cv::VideoCapture &cap){
    const int count_to_read = 100;
    cv::Mat tmp_frame;
    QElapsedTimer timer;
    timer.start();
    for(int i=0; i < count_to_read; i++){
        cap >> tmp_frame;
    }
    int elapsed_ms = timer.elapsed();
    fps = count_to_read / (elapsed_ms / 1000.0);
    fps_calculating = false;
    emit fpsChanged(fps);
}

void CaptureThread::startSavingVideo(cv::Mat &firstFrame){
    saved_video_name = Utilities::newSavedVideoName();

    QString cover = Utilities::getSavedVideoPath(saved_video_name, "jpg");
    cv::imwrite(cover.toStdString(), firstFrame);

    video_writer = new cv::VideoWriter(
                Utilities::getSavedVideoPath(saved_video_name, "mp4").toStdString(),
                cv::VideoWriter::fourcc('M', 'P', '4', 'V'),
                fps? fps:FIXED_FRAME,
                cv::Size(frame_width, frame_height));
    video_saving_status = STARTED;

}

void CaptureThread::stopSavingVideo(){
    video_saving_status = STOPPED;
    video_writer->release();
    delete video_writer;
    video_writer = nullptr;
    emit videoSaved(saved_video_name);
}
