LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE := opencv
#LOCAL_SRC_FILES := ../prebuilt/libopencv_java.so
#include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(LOCAL_PATH)\..\prebuilt\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk
OPENCV_CAMERA_MODULES:=off
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

#include $(CLEAR_VARS)
LOCAL_MODULE    := renderJni
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SRC_FILES := renderer.cpp utils.cpp HDRRenderer.cpp OpenCvJNI.cpp
#LOCAL_SHARED_LIBRARIES := opencv
LOCAL_LDLIBS := -lm -llog -landroid -lEGL -lGLESv3 -lstdc++
include $(BUILD_SHARED_LIBRARY)


#opencv
#include $(CLEAR_VARS)
#include $(LOCAL_PATH)\..\prebuilt\OpenCV-android-sdk\sdk\native\jni\OpenCV.mk
#OPENCV_CAMERA_MODULES:=off
#OPENCV_INSTALL_MODULES:=on
#OPENCV_LIB_TYPE:=SHARED

#LOCAL_ALLOW_UNDEFINED_SYMBOLS := true