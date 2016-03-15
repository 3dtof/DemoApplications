cmake_minimum_required(VERSION 2.8)
find_package(PCL 1.7 REQUIRED COMPONENTS common io visualization)
find_package(OpenCV REQUIRED)

add_definitions(${PCL_DEFINITIONS})
add_definitions(-pthread -std=c++11 -fPIC -ffast-math)

set(VOXEL_INCLUDE_DIRS . /usr/local/include/voxel /usr/local/include/voxel/pcl /usr/local/include/voxel/ti3dtof /usr/local/include/voxel/Filter )
set(VOXEL_LIBRARIES /usr/local/lib/libti3dtof.so /usr/local/lib/libvoxel.so /usr/local/lib/libvoxelpcl.so)

add_executable(SimplePeopleTracking SimplePeopleTracking.cpp Horus.cpp TOFApp.cpp)
target_include_directories(SimplePeopleTracking PUBLIC ${VOXEL_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS} ${EIGEN_INCLUDE_DIRS})
target_link_libraries(SimplePeopleTracking voxelpcl X11 ${OpenCV_LIBS} ${VOXEL_LIBRARIES} ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} ${PCL_VISUALIZATION_LIBRARIES})

IF(LINUX)
  set(CPACK_COMPONENTS_ALL apps)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Voxel sample applications")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libvoxel (>= ${VOXEL_VERSION}), libvoxel-pcl (>= ${VOXEL_VERSION})")
  create_cpack_config(libvoxel-apps ${VOXEL_VERSION})
ELSEIF(WINDOWS)
  
  INSTALL(PROGRAMS VoxelCLIStart.cmd
    DESTINATION bin
    COMPONENT apps
  )
ENDIF()
