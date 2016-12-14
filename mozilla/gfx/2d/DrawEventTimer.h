#ifndef WEBGRAPHICS_GFX_DRAWEVENTTIMER_H_
#define WEBGRAPHICS_GFX_DRAWEVENTTIMER_H_

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <pthread.h>
#include <sys/time.h>
#endif

class DrawEventTimer
{
public:
    void Start()
    {
        if (!mOnceFlag) {
#ifdef WIN32
            ::QueryPerformanceCounter(&mStart);
#else
            gettimeofday(&mStart, NULL);
#endif
            mOnceFlag = true;
            mPrevTimeStamp = Now();
        }
    }

    double Elapse()
    {
        double now = Now();
        double elapse = now - mPrevTimeStamp;
        mPrevTimeStamp = now;
        return elapse;
    }

private:
    double Now()
    {
#ifdef WIN32
        LARGE_INTEGER end, freq;
        ::QueryPerformanceCounter(&end);
        ::QueryPerformanceFrequency(&freq);
        return (double(end.QuadPart) - double(mStart.QuadPart)) / double(freq.QuadPart) * 1000.00;
#else
        struct timeval end;
        gettimeofday(&end, NULL);

        long seconds = end.tv_sec - mStart.tv_sec;
        long useconds = end.tv_usec - mStart.tv_usec;
        long mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
        return mtime;
#endif
    }

#ifdef WIN32
    LARGE_INTEGER mStart;
#else
    struct timeval mStart;
#endif
    bool mOnceFlag { false };
    double mPrevTimeStamp;
};

#endif // WEBGRAPHICS_GFX_DRAWEVENTTIMER_H_
