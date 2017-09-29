
# baseline


<h2>Introduction</h2>

'baseline' is an example project based on Voxel SDK.  It has these components:


<h3>Grabber - (grabber.h, grabber.cpp)</h3> 

	"Wrapper" class over the Voxel::DepthCamera class.  It provide basic TOF camera support. 


<h3>Basic - (basic.h basic.cpp)</h3>

	Example application-specific class built on top of Grabber.  Generally this 
       	class and app.cpp should be customized, while other should left untouched.


<h3>CvDisplay - (cvdisplay.h cvdisplay.cpp)</h3>

	OpenCV GUI interface that can display images and set sliders


<h3>CvUtil - (cvutil.h cvutil.cpp)</h3>

	Simple utility class currently only providing console getkey() function


<h3>app.cpp</h3> 
	Example applicaiton main() containing choice of looped execution or callback execution.



<h2>Compile baseline</h2> 

To build baseline:

	mkdir build
	cd build
	cmake ..
	make
	./app 	 	# run it


<h2>Create your own application based on 'baseline'</h2>

1.  Create your own class derived from Grabber (replacing Basic).  
    Put most, if not all application-specific features in this class.  Do not touch Grabber.

    For instance, if you want to create a handtracking recognizer, replace Basic with HandTracker
    and implement and encapsulate hand-tracking algorithms/data inside this class.

2.  Update main.cpp and CMakeLists.txt as required.
 
3.  Build and run 
