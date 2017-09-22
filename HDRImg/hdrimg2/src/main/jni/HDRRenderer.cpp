#include "com_example_hdrimg2_HDRRenderer.h"
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
static bool useGpu;
static int burnIters;
static int iters;

static double total_init_time;
static double total_upload_img_time;
static double total_upload_coeffs_time;
static double total_glDraw_time;

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

JNIEXPORT void JNICALL Java_com_example_hdrimg2_HDRRenderer_init(JNIEnv *env, jobject, jint imageWidth, jint imageHeight, jint gridWidth, jint gridHeight, jint gridDepth, jstring binFilePath, jint modelType){
    jboolean ptfalse = false;
    const char *cstr = NULL;
    cstr = env->GetStringUTFChars(binFilePath, &ptfalse);
    string rootPath = cstr;

    switch(modelType){
        case 1:
            checkpointPath = rootPath + "/hdrp/";
            break;
        case 2:
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
    useGpu = false;
    burnIters = 2;
    iters = 2;

    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "%s", glGetString(GL_VERSION));
    GLuint vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);

    LOGI("new StandardRenderer start");
    Timer tmpTime;
    tmpTime.start();
    renderer_ = new StandardRenderer(imageWidth, imageHeight, gridWidth, gridHeight, gridDepth, vertexShader, fragmentShader, checkpointPath);
    double costTime = tmpTime.duration();
    total_init_time += costTime;
    LOGE("JNI init: *************************************************new StandardRenderer cost %lf ms", costTime);
    LOGE("JNI init: *************************************************new StandardRenderer total time cost %lf ms", total_init_time);
    LOGI("new StandardRenderer end");
    return;
}

JNIEXPORT void JNICALL Java_com_example_hdrimg2_HDRRenderer_upLoadImg(JNIEnv *env, jobject, jstring path, jint type){
    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "Java_com_example_hdrimg2_HDRRenderer_upLoadImg begin");
    const char *imagePath = NULL;
    jboolean isCopy;
    imagePath = env->GetStringUTFChars(path, &isCopy);
    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "%s", imagePath);

    Timer tmpTime;
    tmpTime.start();
    Mat srcImage = imread(imagePath, CV_LOAD_IMAGE_COLOR);

    double costTime = tmpTime.duration();
    LOGE("JNI upLoadImg: *************************************************imread img cost %lf ms", costTime);

    //这里应该是加载rgb图像
    //if(type == 1)
    if(0)
    {
        Mat YUVImage;
        cvtColor(srcImage, YUVImage, COLOR_BGR2YUV);
        Mat RGBImage;
        cvtColor(whiteBalance(YUVImage), RGBImage, COLOR_YUV2RGB);
        renderer_->upload_input(RGBImage);

        //renderer_->upload_input(whiteBalance(YUVImage));
    }
    else
    {
        Mat RGBImage;
        cvtColor(srcImage, RGBImage, COLOR_BGR2RGB);

        Timer tmpTime;
        tmpTime.start();
        renderer_->upload_input(RGBImage);
        double costTime = tmpTime.duration();
        total_upload_img_time += costTime;
        LOGE("JNI upLoadImg: *************************************************upload_inputImg cost %lf ms", costTime);
        LOGE("JNI upLoadImg: *************************************************upload_inputImg total time cost %lf ms", total_upload_img_time);
    }
    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "Java_com_example_hdrimg2_HDRRenderer_upLoadImg ends");
}

JNIEXPORT jintArray JNICALL Java_com_example_hdrimg2_HDRRenderer_render(JNIEnv *env, jobject, jfloatArray coeffsData, jint srcWidth, jint srcHeight){
    jboolean ptfalse = false;
    jfloat* coeffsBuf = env->GetFloatArrayElements(coeffsData, &ptfalse);
    if(coeffsBuf == NULL)
        return 0;

    //recard time , but no use becase I have commented the part of "glQueryCounter"
    double rendering_gl_coeff;
    double rendering_gl_draw;
    double rendering_gl_readback;

    Mat output(srcHeight, srcWidth, CV_8UC3, cv::Scalar(0));
    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "renderer_->render begin");

    Timer tmpTime;
    tmpTime.start();
    renderer_->render((float *)coeffsBuf, output, &rendering_gl_coeff, &rendering_gl_draw, &rendering_gl_readback);
    double costTime = tmpTime.duration();
    total_upload_coeffs_time += rendering_gl_coeff;
    total_glDraw_time += rendering_gl_draw;
    LOGE("JNI render: *************************************************render cost %lf ms", costTime);
    LOGE("JNI render: *************************************************rendering_gl_coeff cost %lf ms", rendering_gl_coeff);
    LOGE("JNI render: *************************************************total_upload_coeffs_time total time cost %lf ms", total_upload_coeffs_time);
    LOGE("JNI render: *************************************************rendering_gl_draw cost %lf ms", rendering_gl_draw);
    LOGE("JNI render: *************************************************total_glDraw_time total time cost %lf ms", total_glDraw_time);

    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "renderer_->render end");

    //convert RGB to BGRA in order to make bitmap
    //cvtColor(output, output, COLOR_YUV2RGB);
    cvtColor(output, output, COLOR_RGB2BGRA);
    /*
    //if(imwrite("/storage/emulated/0/laplcian.jpg", output))
    if(imwrite("/sdcard/laplcian.jpg", output))
        __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "imwrite success");
    else
        __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "imwrite failed");
*/
    int size = output.rows * output.cols;
    jintArray hdrResult = env->NewIntArray(size);
    env->SetIntArrayRegion(hdrResult, 0, size, (int *)output.data);
    env->ReleaseFloatArrayElements(coeffsData, coeffsBuf, 0);
    return hdrResult;
}

JNIEXPORT jfloatArray JNICALL Java_com_example_hdrimg2_HDRRenderer_getSmallImage(JNIEnv *env, jobject, jstring path, jint modelType){
    const char *imagePath = NULL;
    jboolean isCopy;
    imagePath = env->GetStringUTFChars(path, &isCopy);
    __android_log_print(ANDROID_LOG_INFO, "HDRRenderer.cpp", "%s", imagePath);

    int size = 256;
    Mat image = imread(imagePath, CV_LOAD_IMAGE_COLOR);
    Mat image_yuv, input_lowres;
    resize(image, image, cv::Size(size, size), 0, 0, INTER_NEAREST);
    //LOGE("RGB value:after resize (0,0) R: %d, G: %d, B: %d", (int)image.at<Vec3b>(0,0)[2], (int)image.at<Vec3b>(0,0)[1], (int)image.at<Vec3b>(0,0)[0]);
    //LOGE("RGB value:after resize (1,0) R: %d, G: %d, B: %d", (int)image.at<Vec3b>(1,0)[2], (int)image.at<Vec3b>(1,0)[1], (int)image.at<Vec3b>(1,0)[0]);
    //LOGE("RGB value:after resize (0,1) R: %d, G: %d, B: %d", (int)image.at<Vec3b>(0,1)[2], (int)image.at<Vec3b>(0,1)[1], (int)image.at<Vec3b>(0,1)[0]);

    if(modelType == 1)
    //if(0)
    {
        //the hdrp model is trained by YUV image, so must convert BGR to YUV
        Mat YUVImage, YCrCbImage;
        cvtColor(image, YUVImage, COLOR_BGR2YUV);
        //cvtColor(image, YCrCbImage, COLOR_BGR2YCrCb);
        //cvtColor(whiteBalance(YUVImage), input_lowres, COLOR_YUV2RGB);
        //cvtColor(image, input_lowres, COLOR_BGR2YUV);
        //input_lowres = whiteBalance(YUVImage);

        cvtColor(whiteBalance(image), YCrCbImage, COLOR_BGR2YCrCb);
        input_lowres = YCrCbImage;
    }
    else{
        cvtColor(image, input_lowres, COLOR_BGR2RGB);
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

JNIEXPORT void JNICALL Java_com_example_hdrimg2_HDRRenderer_clearTime(JNIEnv *, jobject){
    total_init_time = 0.0;
    total_upload_img_time = 0.0;
    total_upload_coeffs_time = 0.0;
    total_glDraw_time = 0.0;
}

JNIEXPORT void JNICALL Java_com_example_hdrimg2_HDRRenderer_glesResize(JNIEnv *, jobject, jint w, jint h)
{
    glViewport(0, 0, w, h);
}

JNIEXPORT void JNICALL Java_com_example_hdrimg2_HDRRenderer_freeCurrentState(JNIEnv *, jobject){
    delete renderer_;
    renderer_ = NULL;
}
