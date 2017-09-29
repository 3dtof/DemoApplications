/*! 
 * ===================================================================
 *
 *  @addtogroup	   	baseline	
 *  @{
 *
 *  @file 	basic.cpp
 *
 *  @brief	Basic example class
 *
 *  Copyright (c) 2017 Texas Instruments Inc.
 *
 *====================================================================
 */
#define __BASIC_CPP__
#include "basic.h"

Basic::Basic(DepthCameraPtr dc, FrameFlag flag, CameraSystem &sys) : Grabber(dc, flag, sys)
{
    _init();
}


Basic::~Basic()
{
}


void Basic::update(DepthFrame *frm)
{
    if (frm)
        depthFrameToMat(frm, *getMat("depth"), *getMat("amplitude")); 
}

void Basic::update(ToFRawFrame *frm)
{
    if (frm)
        rawFrameToMat(frm, *getMat("phase"), *getMat("amplitude")); 
}


void Basic::_init()
{
    getFrameSize(_height, _width);  
    std::cout << "width=" << _width << " height=" << _height << std::endl;

    cv::Mat *depth = new cv::Mat(_height, _width, CV_32FC1);
    cv::Mat *phase = new cv::Mat(_height, _width, CV_32FC1);
    cv::Mat *amp   = new cv::Mat(_height, _width, CV_32FC1);

    *depth = cv::Mat::zeros(_height, _width, CV_32FC1);
    *phase = cv::Mat::zeros(_height, _width, CV_32FC1);
    *amp = cv::Mat::zeros(_height, _width, CV_32FC1);

    addMat("depth", depth);
    addMat("phase", phase);
    addMat("amplitude", amp);
}


bool Basic::addMat(std::string name, cv::Mat *mat)
{
    if (!mat)
    {
        std::cout << "Error: addMat() - adding null Mat." << std::endl;
        return false;
    }
    else if (_imageMap.find(name) != _imageMap.end())
    {
        std::cout << "Error: addMat() - " << name << " exists." << std::endl;
        return false;
    }

    _imageMap[name] = mat;
    return true;
}


cv::Mat* Basic::getMat(std::string name)
{
    if (_imageMap.find(name) == _imageMap.end())
    {
        std::cout << "Error: getMat() - " << name << " not found." << std::endl;
        return NULL;
    }
    else
        return _imageMap[name];
}


void Basic::rawFrameToMat(ToFRawFrame *frame, cv::Mat &phase, cv::Mat &amplitude)
{
    for (auto i=0; i<_height; i++ )
    {
        for (auto j=0; j<_width; j++ )
        {
            int idx = i * _width +j;
            if (frame->phaseWordWidth() == 2) 
                phase.at<float>(i,j) = (float)(((uint16_t *)frame->phase())[idx]);
            else if (frame->phaseWordWidth() == 1)
                phase.at<float>(i,j) = (float)(((uint8_t *)frame->phase())[idx]);

            if (frame->amplitudeWordWidth() == 2)
                amplitude.at<float>(i,j) = (float)(((uint16_t *)frame->amplitude())[idx]);
            else if (frame->amplitudeWordWidth() == 1)
                amplitude.at<float>(i,j) = (float)(((uint8_t *)frame->amplitude())[idx]);
        }
    }
#if 0
    std::cout << "phase=" << phase.at<float>(120,160) 
              << " amp= " << amplitude.at<float>(120,160)
              << std::endl;
#endif
}


void Basic::depthFrameToMat(Voxel::DepthFrame *frame, cv::Mat &depth, cv::Mat &amplitude)
{
    bool ret = false;

    for (auto i=0; i<_height; i++ )
    {
        for (auto j=0; j<_width; j++ )
        {
            int idx = i * _width +j;
            depth.at<float>(i,j) = frame->depth[idx];
            amplitude.at<float>(i,j) = frame->amplitude[idx];
        }
    }
#if 0
    std::cout << "depth=" << depth.at<float>(120,160) 
              << " amp= " << amplitude.at<float>(120,160)
              << std::endl;
#endif
}

#undef __BASIC_CPP__
/*! @} */
