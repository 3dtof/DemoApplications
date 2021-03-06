SET(VOXEL_TEST_VERSION ${VOXEL_VERSION})

add_executable(DeviceTest DeviceTest.cpp)
target_link_libraries(DeviceTest voxel)

add_executable(DownloaderTest DownloaderTest.cpp)
target_link_libraries(DownloaderTest voxel)

add_executable(UVCStreamerTest UVCStreamerTest.cpp)
target_link_libraries(UVCStreamerTest voxel)

add_executable(CameraSystemTest CameraSystemTest.cpp)
target_link_libraries(CameraSystemTest voxel)

add_executable(CameraSystemSaveStreamTest CameraSystemSaveStreamTest.cpp)
target_link_libraries(CameraSystemSaveStreamTest voxel)

add_executable(CameraSystemReadStreamTest CameraSystemReadStreamTest.cpp)
target_link_libraries(CameraSystemReadStreamTest voxel)

add_executable(DMLParseTest DMLParseTest.cpp)
target_link_libraries(DMLParseTest voxel)

install(TARGETS
  DeviceTest 
  DownloaderTest 
  UVCStreamerTest
  CameraSystemTest
  CameraSystemSaveStreamTest
  CameraSystemReadStreamTest
  DMLParseTest
  RUNTIME
  DESTINATION bin
  COMPONENT test
)
             
IF(LINUX)
  set(CPACK_COMPONENTS_ALL test)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Voxel library test programs.")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libvoxel (>= ${VOXEL_VERSION})")
  create_cpack_config(libvoxel-test ${VOXEL_TEST_VERSION})
ELSEIF(WINDOWS)
  set(CPACK_COMPONENT_TEST_DESCRIPTION "Voxel Test Applications" PARENT_SCOPE)
  set(CPACK_COMPONENT_TEST_DISPLAY_NAME "Voxel Test" PARENT_SCOPE)
ENDIF()
