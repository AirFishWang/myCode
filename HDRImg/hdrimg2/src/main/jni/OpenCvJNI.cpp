#include "com_example_hdrimg2_OpenCvJNI.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <GLES3/gl3.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#include "utils.h"

using namespace cv;
using namespace std;
static bool flag = true;

#define LOG_TAG "OpenCvJNI.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

JNIEXPORT jintArray JNICALL Java_com_example_hdrimg2_OpenCvJNI_process
(JNIEnv *env, jobject, jintArray bitmapBuff, jint w, jint h){
    jboolean ptfalse = false;
    jint* srcBuf = env->GetIntArrayElements(bitmapBuff, &ptfalse);
    if(srcBuf == NULL)
        return 0;

    int size=w * h;
    Mat srcImage(h, w, CV_8UC4, (unsigned char*)srcBuf);
    Mat tmpIamge = srcImage.clone();

    Mat grayImage;
    cvtColor(srcImage, grayImage, COLOR_BGRA2GRAY);
    cvtColor(grayImage, srcImage, COLOR_GRAY2BGRA);
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, srcBuf);
    env->ReleaseIntArrayElements(bitmapBuff, srcBuf, 0);
    return result;
}