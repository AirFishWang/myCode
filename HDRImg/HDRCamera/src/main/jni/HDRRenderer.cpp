#include "com_example_hdrcamera_HDRGLSurfaceView.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#include "renderer.h"
#include "utils.h"
#include "timer.h"

using namespace cv;
using namespace std;

#define LOG_TAG "HDRRenderer.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static string checkpointPath;
static string shaderRoot;

Renderer *renderer_ = NULL;

Mat whiteBalance(Mat srcImage)
{
    vector<Mat> channels;
    split(srcImage, channels);
    Mat B = channels.at(0);
    Mat G = channels.at(1);
    Mat R = channels.at(2);

    double imageBlueChannelAvg = 0;
    double imageGreenChannelAvg = 0;
    double imageRedChannelAvg = 0;

    //求各通道的平均值
    imageBlueChannelAvg = mean(B)[0];
    imageGreenChannelAvg = mean(G)[0];
    imageRedChannelAvg = mean(R)[0];

    //求出个通道所占增益
    double K = (imageRedChannelAvg + imageGreenChannelAvg + imageRedChannelAvg) / 3;
    double Kb = K / imageBlueChannelAvg;
    double Kg = K / imageGreenChannelAvg;
    double Kr = K / imageRedChannelAvg;

    addWeighted(B, Kb, 0, 0, 0, B);
    addWeighted(G, Kg, 0, 0, 0, G);
    addWeighted(R, Kr, 0, 0, 0, R);

    Mat dstImage;
    merge(channels, dstImage);//图像各通道合并
    return dstImage;
}

void transformMat(Mat &src, Mat &dst, int cameraIndex)  //
{
    Mat transposeImg, mirrorLR, mirrorTB;
    transpose(src, transposeImg);   //转置
    flip(transposeImg, mirrorLR, 1);     //左右翻转，相当于src顺时针转90
    if(cameraIndex == 1)         //前置摄像头采集的图像只需要顺时针转90
    {
        dst = mirrorLR;
        return;
    }else{                       //后置摄像头需要转90度，然后上下翻转
        flip(mirrorLR, mirrorTB, 0);     //上下翻转
        dst = mirrorTB;
        return;
    }

}

JNIEXPORT void JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_init(JNIEnv *env, jobject, jint imageWidth, jint imageHeight, jint gridWidth, jint gridHeight, jint gridDepth, jstring binFilePath, jint cameraIndex, jint modelType){
    jboolean ptfalse = false;
    const char *cstr = NULL;
    cstr = env->GetStringUTFChars(binFilePath, &ptfalse);
    string rootPath = cstr;

    switch(modelType){
        case 1:
            checkpointPath = rootPath + "/hdrp/";
            break;
        case 2:
            //checkpointPath = "/sdcard/hdrmodel/strong_1024/";
            checkpointPath = rootPath + "/normal_1024/";
            break;
        case 3:
            checkpointPath = rootPath + "/faces/";
            break;
        case 4:
            checkpointPath = rootPath + "/early_bird/";
            break;
        default:
            LOGE("Please assign the model type.");
    }
    shaderRoot = rootPath + "/shader/";            //default shader file path
    string vertexShader = shaderRoot + "std.vert";
    string fragmentShader = shaderRoot + "std.frag";
    renderer_ = new StandardRenderer(imageWidth, imageHeight, gridWidth, gridHeight, gridDepth, vertexShader, fragmentShader, checkpointPath, cameraIndex);
    return;
}

JNIEXPORT void JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_upLoadImg(JNIEnv *env, jobject, jintArray rgbBytes, jint width, jint height, jint type){

    jboolean ptfalse = false;
    jint* srcBuf = env->GetIntArrayElements(rgbBytes, &ptfalse);
    if(srcBuf == NULL){
        return;
    }
    int size = width * height;
    Mat srcImage(height, width, CV_8UC4, (unsigned char*)srcBuf);
    Mat RGBImage;
    cvtColor(srcImage, RGBImage, COLOR_BGRA2RGB);
    //if(type == 1)
    if(0)
    {
        //尝试白平衡  没有效果
        Mat YUVImage;
        cvtColor(RGBImage, YUVImage, COLOR_RGB2YUV);
/*
        cvtColor(whiteBalance(YUVImage), RGBImage, COLOR_YUV2RGB);
        renderer_->upload_input(RGBImage);
        */
        renderer_->upload_input(whiteBalance(YUVImage));

    }
    else
    {
        renderer_->upload_input(RGBImage);
    }
}

JNIEXPORT jfloatArray JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_getSmallImage(JNIEnv *env, jobject, jintArray rgbBytes, jint width, jint height, jint cameraIndex, jint type){
    jboolean ptfalse = false;
    jint* srcBuf = env->GetIntArrayElements(rgbBytes, &ptfalse);
    if(srcBuf == NULL){
        return 0;
    }
    Mat srcImage(height, width, CV_8UC4, (unsigned char*)srcBuf);

    int size = 256;
    Mat image, input_lowres;
    resize(srcImage, image, cv::Size(size, size), 0, 0, INTER_NEAREST);
    cvtColor(image, image, COLOR_BGRA2RGB);
    transformMat(image, image, cameraIndex);     //转换的目的是为了与顶点属性的纹理的变换相对应
    //if(modelType == 1)
    if(0)
    {
    //the hdrp model is trained by YUV image, so must convert BGR to YUV
        Mat YUVImage;
        cvtColor(image, YUVImage, COLOR_RGB2YUV);
        //cvtColor(whiteBalance(YUVImage), input_lowres, COLOR_YUV2RGB);
        //cvtColor(image, input_lowres, COLOR_BGR2YUV);
        input_lowres = whiteBalance(YUVImage);
    }
    else{
        input_lowres = image;
    }

    jfloatArray lowres_data = env->NewFloatArray(size*size*3);
    jfloat *farr = env->GetFloatArrayElements(lowres_data, NULL);

    for (int y = 0; y < size; ++y)
        for (int x = 0; x < size; ++x)
            for (int c = 0; c < 3; ++c) {
                farr[c+3*(x+size*y)] = input_lowres.data[c+3*(x+size*y)]/255.0f;
            }
    env->ReleaseFloatArrayElements(lowres_data, farr, 0);
    return lowres_data;
}

JNIEXPORT void JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_render(JNIEnv *env, jobject, jfloatArray coeffsData, jint srcWidth, jint srcHeight){

    //Timer tmpTime;
    jboolean ptfalse = false;
    jfloat* coeffsBuf = env->GetFloatArrayElements(coeffsData, &ptfalse);
    if(coeffsBuf == NULL)
        return;
    //recard time , but no use becase I have commented the part of "glQueryCounter"
    double rendering_gl_coeff;
    double rendering_gl_draw;
    double rendering_gl_readback;

    Mat output(srcHeight, srcWidth, CV_8UC3, cv::Scalar(0));
    //tmpTime.start();
    renderer_->render((float *)coeffsBuf, output, &rendering_gl_coeff, &rendering_gl_draw, &rendering_gl_readback);
    env->ReleaseFloatArrayElements(coeffsData, coeffsBuf, 0);

    //double costTime = tmpTime.duration();
    //__android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "renderer_->render cost time is %lf ",costTime);
}

JNIEXPORT void JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_glesResize(JNIEnv *, jobject, jint w, jint h)
{
    glViewport(0, 0, w, h);
}

JNIEXPORT void JNICALL Java_com_example_hdrcamera_HDRGLSurfaceView_freeCurrentState(JNIEnv *, jobject){
    delete renderer_;
    renderer_ = NULL;
}
