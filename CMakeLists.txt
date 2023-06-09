CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(LittleRenderer)

SET(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

#IF (CMAKE_BUILD_TYPE MATCHES Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBUILD_DEBUG")
#ENDIF()

IF(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    SET(BUILD_SHARED_LIBS TRUE)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WINDOW")
    FIND_LIBRARY(
        VULKAN_LIBRARY
        NAMES vulkan-1 vulkan
        PATHS $ENV{VULKAN_SDK}/lib
        REQUIRED)
    
    IF (VULKAN_LIBRARY)
        INCLUDE_DIRECTORIES($ENV{VULKAN_SDK}/include)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
        MESSAGE("Using system vulkan library")
    ELSE()
        message(FATAL_ERROR "Couldn't find Vulkan library. VULKAN_SDK must be set")
    ENDIF()

    add_definitions(-DASSET_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets")
ELSE() # for android
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")
    ADD_LIBRARY(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    SET(ANDROID_LIBRARIES
        native-app-glue
        android
        log)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_ANDROID")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch arm64")
    SET(VULKAN_LOCATION ${ANDROID_NDK}/sources/third_party/vulkan)
    INCLUDE_DIRECTORIES(${ANDROID_NDK}/sources/android/native_app_glue)
    SET(VULKAN_LIBRARY vulkan)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)

INCLUDE_DIRECTORIES(external/glm)
INCLUDE_DIRECTORIES(external/ktx/include)
INCLUDE_DIRECTORIES(external/tinygltf)
INCLUDE_DIRECTORIES(external/rapidjson)
INCLUDE_DIRECTORIES(src)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(external/ktx)
ADD_SUBDIRECTORY(assets)