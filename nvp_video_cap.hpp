#ifndef __NVP_VIDEO_CAP_H__

#define __NVP_VIDEO_CAP_H__

#define USE_STD                     // Use std lib rather than thread lib

#include <opencv2/opencv.hpp>
#ifdef USE_STD
#include <thread>
#include <mutex>
#else
#include <boost/thread.hpp>
#endif

class NvpVideoCap{
private:
    int             mId;
    cv::VideoCapture *mCap;
    bool            mOpened;

    cv::Mat         mFrames[2];
    int             mCurFrame;
#ifdef USE_STD
    std::mutex      mtx;
    std::thread     *mThread;
#else
    boost::mutex    mtx;
    boost::thread   *mThread;
#endif

    int             mWidth;
    int             mHeight;

    void VideoCapThread()
    {
        bool ret;
        cv::Mat tmp;

        //fprintf (stderr, "VideoCapThread(): in...\n");
        while (mOpened) {
            if (mCap) {
                ret = mCap->read(tmp);
                if (!ret) {
                    fprintf (stderr, "Read frame error!\n"); 
                    Close();
                    break;
                }
                if (mtx.try_lock()) { 
                    mCurFrame ^= 1;
                    tmp.copyTo(mFrames[mCurFrame]);
                    mtx.unlock();
                }
            }
        }
    }

    // Disable copy constructor
    NvpVideoCap(NvpVideoCap &cap);

    // Disable assignment
    NvpVideoCap& operator=(const NvpVideoCap &cap);

public:
    NvpVideoCap() : mCap(NULL), mOpened(false) 
    {
        mId       = 0;
        mCurFrame = 0;
        mWidth    = 0;
        mHeight   = 0;
    }

    ~NvpVideoCap()
    {
        Close();
    }

    bool isOpened()
    {
        return mOpened;
    }

    int GetOneFrame(cv::Mat& img)
    {
        if (!isOpened()) {
            return -1;
        }
        mtx.lock();
        mFrames[mCurFrame].copyTo(img);
        mtx.unlock();
        return 0;
    }

    int Width()
    {
        return mWidth;
    }

    int Height()
    {
        return mHeight;
    }

    void SetFrameWidth(int width)
    {
        mCap->set(CV_CAP_PROP_FRAME_WIDTH,  width);
        mWidth = width;
    }

    void SetFrameHeight(int height)
    {
        mCap->set(CV_CAP_PROP_FRAME_HEIGHT, height);
        mHeight = height;
    }

    void SetFrameRate(int rate)
    {
        mCap->set(CV_CAP_PROP_FPS, rate);
    }

    void Close()
    {
        mOpened = false;
        if (mThread) {
            mThread->join();
        }
        mThread = NULL;

        if (mCap) {
            mCap->release();
        }
        mCap = NULL;
    }

    int Open(int id = 0, int width = 0, int height = 0)
    {
        mId = id;
        fprintf (stderr, "mId is %d, width = %d, height = %d.\n", 
                mId, width, height
                );
        mCap = new cv::VideoCapture(mId);
        if (!mCap->isOpened()) {
            Close();
            return -1;
        }

        if (width != 0 && height != 0) {
            SetFrameWidth(width);
            SetFrameHeight(height);
        }

        mCap->read(mFrames[0]);
        mCap->read(mFrames[1]);

        mOpened = true;
#ifdef USE_STD
        mThread = new std::thread(&NvpVideoCap::VideoCapThread, this);
#else
        mThread = new boost::thread(&NvpVideoCap::VideoCapThread, this);
#endif
        if (mThread == NULL) {
            mOpened = false;
            Close();
            return -1;
        }

        return 0;
    }

};

#endif // #ifndef __NVP_VIDEO_CAP_H__
