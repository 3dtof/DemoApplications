
#Notes on baseline


#Introduction

'baseline' is an example project based on Voxel SDK.  It has these components:


<h2>Grabber - (grabber.h, grabber.cpp)</h2> 

	"Wrapper" class over the Voxel::DepthCamera class.  It provide basic TOF camera support. 


<h2>Basic - (basic.h basic.cpp)</h2>

	Example application-specific class built on top of Grabber.  Generally this 
       	class and app.cpp should be customized, while other should left untouched.


<h2>CvDisplay - (cvdisplay.h cvdisplay.cpp)</h2>

	OpenCV GUI interface that can display images and set sliders


<h2>CvUtil - (cvutil.h cvutil.cpp)</h2>

	Simple utility class currently only providing console getkey() function


<h2>app.cpp</h2> 
	Example applicaiton main() containing choice of looped execution or callback execution.



#Compile baseline 

To build baseline:

	mkdir build
	cd build
	cmake ..
	make
	./app 	 	# run it


#Create your own application based on 'baseline'

1.  Create your own class derived from Grabber (replacing Basic).  
    Put most, if not all application-specific features in this class.  Do not touch Grabber.

    For instance, if you want to create a handtracking recognizer, replace Basic with HandTracker
    and implement and encapsulate hand-tracking algorithms/data inside this class.

2.  Update main.cpp and CMakeLists.txt as required.
 
3.  Build and run 
