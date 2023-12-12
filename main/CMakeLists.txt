file(GLOB_RECURSE SOURCES "src/**.cpp" "src/**.c")

if(CONFIG_CM_BUILD_FIRMWARE)
    set(ENTRY "main.cpp")
endif()

file(GLOB_RECURSE LIBS "lib/*/src/**.cpp" "lib/*/src/**.c")
set(LIBS_INCL "lib/glm/glm" "lib/dr_libs" "lib/opencv" "lib/libhelix-aac/src")

idf_component_register(SRCS ${ENTRY} ${SOURCES} ${LIBS} INCLUDE_DIRS "src" ${LIBS_INCL})

add_prebuilt_library(opencv_imgproc "lib/opencv/libopencv_imgproc.a" PRIV_REQUIRES cxx)
add_prebuilt_library(opencv_core "lib/opencv/libopencv_core.a" PRIV_REQUIRES cxx)
target_link_libraries(${COMPONENT_LIB} opencv_imgproc opencv_core)

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
