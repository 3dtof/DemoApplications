/*
 * TI Grabber component.
 *
 * Copyright (c) 2017 Texas Instruments Inc.
 */

#include "grabber.h"

#define FIFO_SIZE       3


/*!
 *=============================================================================
 *
 * \brief Grabber::Grabber
 * \param depthCamera
 * \param frameFlag
 * \param sys
 *
 *=============================================================================
 */
Grabber::Grabber(DepthCameraPtr depthCamera, FrameFlag frameFlag, CameraSystem &sys) :
        _depthCamera(depthCamera), _frameFlag(frameFlag), _sys(sys)
{  
    if (!_depthCamera->isInitialized())
    {
        logger(LOG_ERROR) << "Grabber: camera not initialized." << endl;
        return;
    }

    FrameSize sz;
    _depthCamera->getFrameSize(sz);
    
    _rows = sz.height;
    _cols = sz.width;
    
    _frameCount = 0;
    
    _updateDone = false;
    
    setFrameRate(30.0);

    if (_frameFlag & FRAMEFLAG_XYZI_POINT_CLOUD_FRAME)
        _depthCamera->registerCallback(DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME,
            std::bind(&Grabber::_callback, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    if (_frameFlag & FRAMEFLAG_DEPTH_FRAME)
        _depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME,
            std::bind(&Grabber::_callback, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    if (_frameFlag & FRAMEFLAG_RAW_PROCESSED)
        _depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_PROCESSED,
            std::bind(&Grabber::_callback, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    if (_frameFlag & FRAMEFLAG_RAW_UNPROCESSED)
        _depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_UNPROCESSED,
            std::bind(&Grabber::_callback, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

}



/*!
 *=============================================================================
 *
 * \brief Grabber::getFrameCount
 * \return
 *
 *=============================================================================
 */
uint32_t Grabber::getFrameCount()
{ 
    Lock<Mutex> _(_mtx);
    
    return _frameCount;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::getDepthFrame
 * \return
 *
 *=============================================================================
 */
DepthFrame * Grabber::getDepthFrame()
{
    DepthFrame *frame = NULL;

    Lock<Mutex> _(_mtx);
 
    if (_qDepthFrame.size() > 0 && (_frameFlag & Grabber::FRAMEFLAG_DEPTH_FRAME) )
    {
        frame = _qDepthFrame.front();
        _qDepthFrame.pop_front();
    }

    return frame;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::getXYZIFrame
 * \return
 *
 *=============================================================================
 */
XYZIPointCloudFrame * Grabber::getXYZIFrame()
{
    XYZIPointCloudFrame *frame = NULL;

    Lock<Mutex> _(_mtx);
 
    if (_qXYZIFrame.size() > 0 && (_frameFlag & FRAMEFLAG_XYZI_POINT_CLOUD_FRAME) )
    {
        frame = _qXYZIFrame.front();
        _qXYZIFrame.pop_front();
	}

    return frame;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::getRawFrameProcessed
 * \return
 *
 *=============================================================================
 */
Ptr<Frame> Grabber::getRawFrameProcessed()
{
    Ptr<Frame> frame = NULL;

    Lock<Mutex> _(_mtx);

    if (_qProcessedFrame.size() > 0 && (_frameFlag & FRAMEFLAG_RAW_PROCESSED) )
    {
        frame = _qProcessedFrame.front();
        _qProcessedFrame.pop_front();
    }

    return frame;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::getRawFrameUnprocessed
 * \return
 *
 *=============================================================================
 */
Ptr<Frame> Grabber::getRawFrameUnprocessed()
{
    Ptr<Frame> frame = NULL;

    Lock<Mutex> _(_mtx);

    if (_qUnprocessedFrame.size() > 0 && (_frameFlag & FRAMEFLAG_RAW_UNPROCESSED) )
    {
        frame = _qUnprocessedFrame.front();
        _qUnprocessedFrame.pop_front();
    }

    return frame;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::setFrameRate
 * \param frameRate
 *
 *=============================================================================
 */
void Grabber::setFrameRate(float frameRate)
{
    FrameRate r;
    
    r.denominator = 10000;
    r.numerator = int(frameRate)*10000;
    int g = gcd(r.numerator, r.denominator);
    r.numerator /= g;
    r.denominator /= g;
    
    _depthCamera->setFrameRate(r);
}


/*!
 *=============================================================================
 *
 * \brief Grabber::registerUpdate
 * \param update
 *
 *=============================================================================
 */
void Grabber::registerUpdate(std::function<void(Grabber *g, void *ptr)> update, void *ptr)
{
    _update = update;
    _userPtr = ptr;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::run
 *
 *=============================================================================
 */
void Grabber::run()
{
    start();
    
    _updateDone = false;
    
    while (!_updateDone)
    {
        _update(this, _userPtr);
    }
}


/*!
 *=============================================================================
 *
 * \brief Grabber::runExit
 * \return
 *
 *=============================================================================
 */
void Grabber::runExit()
{
    _updateDone = true;
}

/*!
 *=============================================================================
 *
 * \brief Grabber::getProfiles
 * \return
 *
 *=============================================================================
 */
const Map<int, Voxel::String> &Grabber::getProfiles()
{
    return _depthCamera->getCameraProfileNames();
}



/*!
 *=============================================================================
 *
 * \brief Grabber::getCurrentProfileName
 * \return
 *
 *=============================================================================
 */
Voxel::String Grabber::getCurrentProfileName()
{
    int id = getCurrentProfileID();
    const Map<int, Voxel::String> &profiles =
                            _depthCamera->getCameraProfileNames();
    for (auto &p: profiles)
    {
        if (p.first == id)
        {
            Voxel::String name = p.second;
            return name;
        }
    }

    return NULL;
}



/*!
 *=============================================================================
 *
 * \brief Grabber::setProfile
 * \param name
 * \return
 *
 *=============================================================================
 */
bool Grabber::setProfile(Voxel::String name)
{
    bool rc = false;
    const Map<int, Voxel::String> &profiles = 
                            _depthCamera->getCameraProfileNames();
    for (auto &p: profiles) 
    {
        std::cout << "Checking (" << p.second << ") against name (" << name << ")" << std::endl;

        if (p.second == name) 
        {
            int profile_id = p.first;
            
            ConfigurationFile *c = 
                _depthCamera->configFile.getCameraProfile(p.first);
                
            if (c && c->getLocation() == ConfigurationFile::IN_CAMERA) 
            {
                if (_depthCamera->setCameraProfile(profile_id)) 
                {
                    rc = true;
                    break;
                }
            }
        }
    }
    return rc;
}


/*!
 *=============================================================================
 *
 * \brief Grabber::getSerialNumber
 * \param str
 * \return
 *
 *=============================================================================
 */
bool Grabber::getSerialNumber(std::string &str)
{
    return _depthCamera->getSerialNumber(str);
}


/*!
 *=============================================================================
 *
 * \brief Grabber::_callback
 * \param depthCamera
 * \param frame
 * \param type
 *
 *=============================================================================
 */
void Grabber::_callback(DepthCamera &depthCamera, const Frame &frame, 
				DepthCamera::FrameType type)
{
    Lock<Mutex> _(_mtx);

    if (type == DepthCamera::FRAME_DEPTH_FRAME)
    {               
        if (_qDepthFrame.size() >= FIFO_SIZE)
        {
           DepthFrame *f = _qDepthFrame.front();
           _qDepthFrame.pop_front();
           if (f != NULL)
               delete f;
        }

        DepthFrame *nf = new DepthFrame;
        *nf = *dynamic_cast<const DepthFrame *>(&frame);
        _qDepthFrame.push_back(nf);

        _frameCount++;
    }
    else if(type == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME)
    {
        if (_qXYZIFrame.size() >= FIFO_SIZE)
        {
            XYZIPointCloudFrame *f = _qXYZIFrame.front();
            _qXYZIFrame.pop_front();
            if (f != NULL)
                delete f;
        }

        XYZIPointCloudFrame *nf = new XYZIPointCloudFrame;
        *nf = *dynamic_cast<const XYZIPointCloudFrame *>(&frame);
        _qXYZIFrame.push_back(nf);

        _frameCount++;
    }

    else if(type == DepthCamera::FRAME_RAW_FRAME_UNPROCESSED)
    {
        if (_qUnprocessedFrame.size() >= FIFO_SIZE)
        {
            Ptr<Frame> f = _qUnprocessedFrame.front();
            _qUnprocessedFrame.pop_front();
            if (!f.get())
                delete f.get();
        }

        RawDataFrame *nf = dynamic_cast<const RawDataFrame *>(&frame);
        if (nf)
        {
            Ptr<Frame> f2 = nf->copy();
            _qUnprocessedFrame.push_back(f2);
        }

        _frameCount++;
    }

    else if (type == DepthCamera::FRAME_RAW_FRAME_PROCESSED)
    {
        if (_qProcessedFrame.size() >= FIFO_SIZE)
        {
            Ptr<Frame> f = _qProcessedFrame.front();
            _qProcessedFrame.pop_front();
            if (!f.get())
                delete f.get();
        }

        ToFRawFrame *nf = dynamic_cast<const ToFRawFrame *>(&frame);
        if (nf)
        {
            Ptr<Frame> f2 = nf->copy();
            _qProcessedFrame.push_back(f2);
        }

        _frameCount++;
    }
    else 
    {
        logger(LOG_ERROR) << "Grabber: unknown callback type: " << type << endl;
    }
}



