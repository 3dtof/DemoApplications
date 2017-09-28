/*! 
 * ==========================================================================================
 *
 * @addtogroup	   	baseline	
 * @{
 *
 * @file		    cvdisplay.cpp
 * @version		    1.0
 * @date		    9/27/2017
 *
 * Copyright(c) 2017-2018 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ==========================================================================================
 */
#define __CVDISPLAY_CPP__

#include "cvdisplay.h"


static void onTrackbar(int val, void *p)
{
   paramType *t = (paramType *)p;
   float *fval = std::get<0>(*t);
   *fval = (float)val/(float)std::get<3>(*t);
}


CvDisplay::CvDisplay()
{
   _param.clear();
   _image.clear();
   _sliderData.clear();
   cv::namedWindow("Sliders", cv::WINDOW_NORMAL);
}


bool CvDisplay::addParam(std::string name, float *param, float maxVal, int div)
{
    if (_param.find(name) != _param.end())
        return false;

    int *iparam = new int((int)(*param * div));
    int imax = (int)(maxVal * div);
    _param[name] = paramType(param, iparam, imax, div);

    return true;
}


bool CvDisplay::addSlider(std::string name)
{
   if (_param.find(name) == _param.end())
      return false;
   
   cv::createTrackbar(name, std::string("Sliders"), std::get<1>(_param[name]), 
            std::get<2>(_param[name]), onTrackbar, (void *)&_param[name]);
   return true;
}


bool CvDisplay::addImage(std::string name, cv::Mat *img, enum cv::WindowFlags flag)
{
    if (_image.find(name) != _image.end())
        return false;

    cv::namedWindow(name, flag);
    _image[name] = img;
    return true;
}


bool CvDisplay::showImage(std::string name, float gain)
{
    if (_image.find(name) == _image.end())
        return false;

    cv::imshow(name, gain*(*_image[name]));
    return true;
}

bool CvDisplay::showAllImages()
{
    for (auto it = _image.begin(); it != _image.end(); it++)
    {
        std::string name = it->first;
        cv::imshow(name, *_image[name]);
    }
    return true;
}


CvDisplay::~CvDisplay()
{
   
}

#undef __CVDISPLAY_CPP__
/*! @} */
