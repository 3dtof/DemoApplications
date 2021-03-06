cmake_minimum_required (VERSION 2.8.11)
project (libvoxelpcl)

set(VOXEL_PCL_MAJOR_VERSION 0)
set(VOXEL_PCL_MINOR_VERSION 3)
set(VOXEL_PCL_PATCH_VERSION 13)
set(VOXEL_PCL_VERSION ${VOXEL_PCL_MAJOR_VERSION}.${VOXEL_PCL_MINOR_VERSION}.${VOXEL_PCL_PATCH_VERSION})
set(VOXEL_VERSION ${VOXEL_PCL_VERSION})
set(VOXEL_PCL_ABI_VERSION 16)

### Do not export any symbol by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
include(GenerateExportHeader)


set(CMAKE_INCLUDE_CURRENT_DIR 1)

MESSAGE(${CMAKE_SYSTEM_NAME})

set(ARCH ${CMAKE_SYSTEM_PROCESSOR})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  SET(LINUX 1)
  add_definitions(-msse2 -pthread -std=c++11)
  ADD_DEFINITIONS(-DLINUX)  

  if(ARCH STREQUAL "x86_64")
    set(ARCH amd64)
  endif()
  
  file(WRITE "${CMAKE_BINARY_DIR}/make_deb.sh" "for i in CPackConfig-*.cmake; do cpack -G DEB --config $i; done\n")
  
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  SET(WINDOWS 1)
  ADD_DEFINITIONS(-DWINDOWS /wd4251)
endif()

### Find and use Voxel library
find_package(Voxel ${VOXEL_VERSION} REQUIRED)

include_directories(
  ${VOXEL_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/VoxelPCL
)

find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

IF(LINUX)
  function(create_cpack_config name ver)
    set(CPACK_PACKAGE_VERSION "${ver}")
    set(CPACK_PACKAGE_NAME "${name}")
    set(CPACK_PACKAGE_FILE_NAME "${name}-${ver}-${ARCH}")
    set(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackConfig-${name}.cmake")
    include(CPack)
  endfunction(create_cpack_config)
  
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${ARCH})
  set(CPACK_DEBIAN_PACKAGE_SECTION "Electronics")
  set(CPACK_DEB_COMPONENT_INSTALL 1)
ENDIF()

INCLUDE(common.cmake)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
  install(CODE "execute_process(COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_BINARY_DIR})"
    COMPONENT pcl_dev)
  
  if(LINUX)
    install(DIRECTORY 
      ${CMAKE_BINARY_DIR}/doc/html/ 
      DESTINATION share/doc/voxelpcl-${VOXEL_PCL_VERSION}
      COMPONENT pcl_dev)
  elseif(WINDOWS)
  install(DIRECTORY 
      ${CMAKE_BINARY_DIR}/doc/html/ 
      DESTINATION doc/voxelpcl
      COMPONENT pcl_dev)
  endif()
endif()
  
#### Packaging related information -- common to both platforms
set(CPACK_PACKAGE_NAME "Voxel SDK")
set(CPACK_PACKAGE_VERSION ${VOXEL_VERSION})
set(CPACK_PACKAGE_CONTACT "prasad.bhat@ti.com")
set(CPACK_PACKAGE_VENDOR "Texas Instruments, Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Voxel library package. Contains the main Voxel library.")
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

add_subdirectory(VoxelPCL)

create_cmake_config(VoxelPCL "voxelpcl" pcl_dev VoxelPCL pcl)

IF(WINDOWS)
  SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
ENDIF()
