/*! 
 * ==========================================================================================
 *
 * @addtogroup	   	baseline	
 * @{
 *
 * @file		    cvdisplay.h
 * @version		    1.0
 * @date		    9/27/2017
 *
 * Copyright(c) 2017-2018 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ==========================================================================================
 */
#ifndef __CVDISPLAY_H__
#define __CVDISPLAY_H__

#include <unistd.h>
#include <string>
#include <opencv2/opencv.hpp>

// param, iparam, imax, div
typedef std::tuple<float*, int*, int, int> paramType;

class CvDisplay
{
public:
    CvDisplay();
    ~CvDisplay();

    bool addParam(std::string name, float *param, float maxVal, int div);
    bool addSlider(std::string name);
    bool addImage(std::string name, cv::Mat *img, enum cv::WindowFlags flag = cv::WINDOW_NORMAL);
    bool showImage(std::string name, float gain=1.0);
    bool showAllImages();

private:
    std::map< std::string, paramType > _sliderData;
    std::map< std::string, cv::Mat * > _image;
    std::map< std::string, paramType > _param;
    cv::Mat _sliderMat;
};


#endif // __CVDISPLAY_H__
/*! @} */
