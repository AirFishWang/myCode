LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := opencv
LOCAL_SRC_FILES := ../prebuilt/libopencv_java.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := renderJni
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SRC_FILES := renderer.cpp utils.cpp HDRRenderer.cpp yuv2rgb.cpp rgb2yuv.cpp imageUtils_jni.cpp
LOCAL_SHARED_LIBRARIES := opencv
LOCAL_LDLIBS := -lm -llog -landroid -lEGL -lGLESv3 -lstdc++
include $(BUILD_SHARED_LIBRARY)