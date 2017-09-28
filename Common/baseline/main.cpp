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
float ampGain = 1.0;
float phaseGain = 0.1;

#if 1  // Looped

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
    Basic eye(dc, Grabber::FRAMEFLAG_DEPTH_FRAME, sys);
 
    disp.addImage("amplitude", eye.getAmpMat());
    disp.addImage("phase", eye.getPhaseMat());
    disp.addParam("ampGain", &ampGain, 200.0, 100);
    disp.addSlider("ampGain");
    disp.addParam("phaseGain", &phaseGain, 0.5, 1000);
    disp.addSlider("phaseGain");

    if (eye.isInitialized())
    {
        std::cout << "starting camera" << std::endl;
        eye.start();

        while (!done)
        {
            DepthFrame *frame = eye.getDepthFrame();

            if (frame != NULL)
            {
               // std::cout << "Got frame #" << eye.getFrameCount() << std::endl;
                eye.update(frame);
                disp.showImage("amplitude", ampGain);
                disp.showImage("phase", phaseGain);
                delete frame;
            }

            if (util.getkey() == 'q') 
            {
                if (eye.isRunning())
                    eye.stop();
                done = true;
            }
            cv::waitKey(33);
        }
   }

err_exit:
   eye.stop();
}

#else  // Callback based

void myUpdate(Grabber *grabber, void *ptr)
{
    Basic *eye = (Basic *)ptr;
    DepthFrame *frame = grabber->getDepthFrame();
    eye->update(frame);
    disp.showImage("amplitude", ampGain);
    disp.showImage("phase", phaseGain);
    delete frame;
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
    Basic eye(dc, Grabber::FRAMEFLAG_DEPTH_FRAME, sys);
 
    // Create display
    disp.addImage("amplitude", eye.getAmpMat());
    disp.addImage("phase", eye.getPhaseMat());
    disp.addParam("ampGain", &ampGain, 200.0, 100);
    disp.addSlider("ampGain");
    disp.addParam("phaseGain", &phaseGain, 0.5, 1000);
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

