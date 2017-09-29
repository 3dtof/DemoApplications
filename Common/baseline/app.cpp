/*! 
 * ===================================================================
 *
 *  @addtogroup	   	baseline	
 *  @{
 *
 *  @file 	main.cpp
 *
 *  @brief	Basic example program
 *
 *  Copyright (c) 2017 Texas Instruments Inc.
 *
 *====================================================================
 */
#include "basic.h"
#include "cvutil.h"
#include "cvdisplay.h"

CvDisplay disp;
float ampGain = 0.005;
float phaseGain = 0.001;
float depthGain = 0.5;

#if 0  // Looped execution

int main(int argc, char *argv[])
{
    int key;
    bool done = false;
    CvUtil util;

    Voxel::CameraSystem sys;
    std::vector< DevicePtr > devices = sys.scan();
    if (devices.size() <= 0)
    {
        std::cout << "Error: No camera." << std::endl;
        exit (-1);
    }
    DepthCameraPtr dc = sys.connect(devices[0]);
    Basic eye(dc, Grabber::FRAMEFLAG_ALL, sys);

   
    if (!eye.setProfile("MetrilusLongRange"))
    {
        std::cout << "Error: cannot set MetrilusLongRangeProfile." << std::endl;
        exit(-1);
    }

    disp.addImage("amplitude", eye.getMat("amplitude"));
    disp.addImage("depth", eye.getMat("depth"));
    disp.addImage("phase", eye.getMat("phase"));

    disp.addParam("ampGain", &ampGain, 0.01, 10000);
    disp.addSlider("ampGain");
    disp.addParam("depthGain", &depthGain, 1.0, 10000);
    disp.addSlider("depthGain");
    disp.addParam("phaseGain", &phaseGain, 0.01, 10000);
    disp.addSlider("phaseGain");

    if (eye.isInitialized())
    {
        std::cout << "starting camera" << std::endl;
        eye.start();

        while (!done)
        {
            // Get depth frame
            DepthFrame *depthFrame = eye.getDepthFrame();
            if (depthFrame) 
            {        
                eye.update(depthFrame);
                //disp.showImage("amplitude", ampGain);
                disp.showImage("depth", depthGain);
                delete depthFrame;
            }

            // Get raw processed frame
            Ptr<Frame> f = eye.getRawFrameProcessed();
            if (f && f.get())
            {
                ToFRawFrame *frame = dynamic_cast<ToFRawFrame *>(f.get());
                if (frame) 
                {        
                    eye.update(frame);
                    disp.showImage("amplitude", ampGain);
                    disp.showImage("phase", phaseGain);
                }
            }

            char key = cv::waitKey(1);  // Select image when enter keystroke
            if (key == 'q')
            {   
                done = true;
                eye.runExit();
            }
        }
   }

err_exit:
   eye.stop();
}

#else  // Callback execution

void myUpdate(Grabber *grabber, void *ptr)
{
    Basic *eye = (Basic *)ptr;

    // Get depth frame
    DepthFrame *depthFrame = grabber->getDepthFrame();
    if (depthFrame) 
    {        
        eye->update(depthFrame);
        //disp.showImage("amplitude", ampGain);
        disp.showImage("depth", depthGain);
        delete depthFrame;
    }

    // Get raw processed frame
    Ptr<Frame> f = grabber->getRawFrameProcessed();
    if (f && f.get())
    {
        ToFRawFrame *frame = dynamic_cast<ToFRawFrame *>(f.get());
        if (frame) 
        {        
            eye->update(frame);
            disp.showImage("amplitude", ampGain);
            disp.showImage("phase", phaseGain);
        }
    }

    char key = cv::waitKey(1);  // Select image when enter keystroke
    if (key == 'q')
        eye->runExit();

}


int main(int argc, char *argv[])
{
    int key;
    bool done = false;
    CvUtil util;

    // Connect to TOF camera
    Voxel::CameraSystem sys;
    std::vector< DevicePtr > devices = sys.scan();
    if (devices.size() <= 0)
    {
        std::cout << "Error: No camera." << std::endl;
        exit (-1);
    }
    DepthCameraPtr dc = sys.connect(devices[0]);
    Basic eye(dc, Grabber::FRAMEFLAG_ALL , sys);

    if (!eye.setProfile("MetrilusLongRange"))
    {
        std::cout << "Error: cannot set MetrilusLongRangeProfile." << std::endl;
        exit(-1);
    }

    // Create display
    disp.addImage("amplitude", eye.getMat("amplitude"));
    disp.addImage("depth", eye.getMat("depth"));
    disp.addImage("phase", eye.getMat("phase"));

    disp.addParam("ampGain", &ampGain, 0.01, 10000);
    disp.addSlider("ampGain");
    disp.addParam("depthGain", &depthGain, 1.0, 10000);
    disp.addSlider("depthGain");
    disp.addParam("phaseGain", &phaseGain, 0.01, 10000);
    disp.addSlider("phaseGain");

    // Start camera
    if (eye.isInitialized())
    {
        eye.registerUpdate(myUpdate, (void *)&eye);

        std::cout << "starting camera" << std::endl;
        eye.run();
    }

err_exit:
    eye.stop();
}

#endif

