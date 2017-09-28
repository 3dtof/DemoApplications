
Notes on baseline


== Introduction

'baseline' is an example project based on Voxel SDK.  It has three major components:

Grabber - (grabber.h, grabber.cpp) 

	"Wrapper" class over the Voxel::DepthCamera class.  It provide basic TOF camera support. 

Basic - (basic.h basic.cpp)

	Example application-specific class that build on top of Grabber.

CvDisplay - (cvdisplay.h cvdisplay.cpp)

	OpenCV GUI interface that can display images and set sliders


CvUtil - (cvutil.h cvutil.cpp)

	Simple utility class currently only providing console getkey() function

main.cpp - example main().


== Compile baseline 

To build baseline:

	mkdir build
	cd build
	cmake ..
	make
	./main 	 	# run it


== Create your own application based on 'baseline'

1.  Create your own class derived from Grabber (replacing Basic).  
    Put most, if not all application-specific features in this class.  Do not touch Grabber.

    For instance, if you want to create a handtracking recognizer, replace Basic with HandTracker
    and implement and encapsulate hand-tracking algorithms/data inside this class.

2.  Update main.cpp and CMakeLists.txt as required.
 
3.  Build and run 
