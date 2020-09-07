#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H
#include <QString>
#include <QThread>
#include <QMutex>
#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video/background_segm.hpp"


using namespace std;

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    CaptureThread(int camera, QMutex *lock);
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();
    void setRunning(bool run) {running = run; };
    void startCalcFPS() {fps_calculating = true; };
    enum VideoSavingStatus {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };

    void setVideoSavingStatus(VideoSavingStatus status){
        video_saving_status = status;
    };

protected:
    void run() override;

signals:
    void frameCaptured(cv::Mat *data);
    void fpsChanged(float fps);
    void videoSaved(QString name);

private:
    bool running;
    int cameraID;
    QString videoPath;
    QMutex *data_lock;
    cv::Mat frame;

    int cap_count;

    int CAP_MIN = 30; //min
    int FIXED_FRAME = 5;

    //fps
    bool fps_calculating;
    float fps;
    void calculateFPS(cv::VideoCapture &cap);

    // video saving
    int frame_width, frame_height;
    VideoSavingStatus video_saving_status;
    QString saved_video_name;
    cv::VideoWriter *video_writer;
    void startSavingVideo(cv::Mat &firstFrame);
    void stopSavingVideo();

};




#endif // CAPTURE_THREAD_H
