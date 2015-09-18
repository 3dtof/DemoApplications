# CalculusDemo
Prerelease Calculus demo

=Prerequisits:

* Point Cloud Library
	$ sudo add-apt-repository ppa:v-launchpad-jochen-sprickerhof-de/pcl
	$ sudo apt-get update
	$ sudo apt-get install libpcl-all
* USB stack
	$ sudo apt-get install libusb-1.0-0-dev libudev-dev linux-libc-dev
* Build tools:
	$ sudo apt-get install python cmake git g++ swig3.0
* Doxgen and Graphviz
	$ sudo apt-get install doxygen graphviz
* Voxel SDK (with demo code; login required)
  	$ git clone https://github.com/3dtof/CalculusDemo.git

=Build:
  $ cd 3dtof/CalculusDemo
  $ source setup.sh
  $ cd voxel-sdk
  
There are four subdirectories, libvoxel, libti3dtof, libvoxelpcl, libvoxel-app; each require seperate builds. To build them, simple repeat the below steps for each, in the order I just listed, except libvoxel-app; for that, replace 'sudo make install' with 'make':

  $ cd <directory>; mkdir build; cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
  $ sudo make install

The Calculus demo code are under libvoxel-app/build/App.  Most of them require you to run with 'sudo'.  The RobotDemo is the vacuum cleaner demo, the TVDemo require USB-UIRT (www.usbuirt.com).
  
