/*
 * TOFViewer component.
 *
 * Copyright (c) 2017 Texas Instruments Inc.
 */

#ifndef GRABBER_H
#define GRABBER_H

#include <deque>
#include <string>
#include <CameraSystem.h>
#include <DepthCamera.h>
#include <Common.h>
#include <unistd.h>
#include <termio.h>
#include <functional>
#include <stdint.h>

using namespace std;
using namespace Voxel;

class Grabber
{
public:

    typedef enum {
        FRAMEFLAG_RAW_UNPROCESSED = (1<<DepthCamera::FRAME_RAW_FRAME_UNPROCESSED),
        FRAMEFLAG_RAW_PROCESSED = (1<<DepthCamera::FRAME_RAW_FRAME_PROCESSED),
        FRAMEFLAG_DEPTH_FRAME = (1<<DepthCamera::FRAME_DEPTH_FRAME),
        FRAMEFLAG_XYZI_POINT_CLOUD_FRAME = (1<<DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME),
        FRAMEFLAG_ALL = (FRAMEFLAG_RAW_UNPROCESSED
                        | FRAMEFLAG_RAW_PROCESSED
                        | FRAMEFLAG_DEPTH_FRAME
                        | FRAMEFLAG_XYZI_POINT_CLOUD_FRAME)
    } FrameFlag;

protected:
    DepthCameraPtr _depthCamera;
    
    CameraSystem &_sys;

    Mutex _mtx;
    
    deque<DepthFrame* > _qDepthFrame;
    deque<XYZIPointCloudFrame* > _qXYZIFrame;
    deque< Ptr<Frame> > _qProcessedFrame;
    deque< Ptr<Frame> > _qUnprocessedFrame;
    
    FrameFlag _frameFlag;

    uint32_t _frameCount;  
    
    bool _updateDone;
    
    int _rows, _cols;
    
    std::function<void(Grabber *g, void *ptr)> _update;
    void * _userPtr;

    std::vector<FilterPtr> _RawUnprocessedFilters;
    std::vector<FilterPtr> _RawProcessedFilters;
    std::vector<FilterPtr> _DepthFilters;

protected:
    void _callback(DepthCamera &depthCamera, const Frame &frame, 
            DepthCamera::FrameType type);

    void _applyFilter();

public:
    Grabber(DepthCameraPtr depthCamera, FrameFlag flag, CameraSystem &sys);

    virtual float getFramesPerSecond() const
    {
        FrameRate r;
        if(!_depthCamera->getFrameRate(r))
      	    return 0;
    	else
      	    return r.getFrameRate();
  	}

    virtual Vector<String> getSupportedFilters() { return _sys.getSupportedFilters(); }

    virtual FilterPtr createFilter(String name, DepthCamera::FrameType frameType)
            { return _sys.createFilter(name, frameType); }

    virtual int addFilter(FilterPtr p, DepthCamera::FrameType frameType, int beforeId=-1)
            { return _depthCamera->addFilter(p, frameType, beforeId); }

    virtual FilterPtr getFilter(int filterId, DepthCamera::FrameType frameType) const
            { return _depthCamera->getFilter(filterId, frameType); }

    virtual bool removeFilter(int filterId, DepthCamera::FrameType frameType)
            { return _depthCamera->removeFilter(filterId, frameType); }

    virtual bool removeAllFilters(DepthCamera::FrameType frameType)
            { return _depthCamera->removeAllFilters(frameType); }

    virtual void resetFilters() { _depthCamera->resetFilters(); }

    virtual const FilterSet<RawFrame> &getUnprocessedRawFilterSet()
            { return _depthCamera->getUnprocessedRawFilterSet(); }

    virtual const FilterSet<RawFrame> &getProcessedRawFilterSet()
            { return _depthCamera->getProcessedRawFilterSet(); }

    virtual const FilterSet<DepthFrame> &getDepthFilterSet()
            { return _depthCamera->getDepthFilterSet(); }

    virtual DepthCameraPtr getDepthCamera() { return _depthCamera; }

    virtual std::string getName() const { return _depthCamera->id(); }

    virtual bool isRunning() const { return _depthCamera->isRunning(); }

    virtual bool isInitialized() const { return _depthCamera->isInitialized(); }

    virtual void start() { _depthCamera->start(); }

    virtual void stop() { _depthCamera->stop(); _depthCamera->wait(); }

    virtual uint32_t getFrameCount();

    virtual FrameFlag getFrameFlag() { return _frameFlag; }
    
    virtual XYZIPointCloudFrame * getXYZIFrame();
    
    virtual DepthFrame * getDepthFrame();

    virtual Ptr<Frame> getRawFrameProcessed();

    virtual Ptr<Frame> getRawFrameUnprocessed();

    virtual void setFrameRate(float frameRate);
    
    virtual void getFrameSize(int &rows, int &cols) { rows=_rows; cols=_cols; }

    bool getSerialNumber(std::string &str);

    virtual const Map<int, Voxel::String> &getProfiles();

    virtual int getCurrentProfileID() { return _depthCamera->getCurrentCameraProfileID(); }

    virtual Voxel::String getCurrentProfileName();

    virtual bool setProfile(Voxel::String name);

    virtual Ptr< RegisterProgrammer > getProgrammer() { return _depthCamera->getProgrammer(); }

    virtual Map<String, ParameterPtr> getParameters() { return _depthCamera->getParameters(); }

    virtual bool setRegister(Voxel::String name, uint value)
                    { return _depthCamera->set(name, value); }
                        
    virtual void registerUpdate(std::function<void(Grabber *g, void *ptr)> update, void *ptr);
    
    virtual void updateExit() { if (isRunning()) stop(); _updateDone = true; }
    
    virtual void run();

    virtual void runExit();

    virtual ~Grabber()
    {
        if(isRunning())
            stop();
    }

};


#endif // GRABBER_H
