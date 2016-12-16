#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

#define WINDOW_NAME "【线性混合示例】"

const int g_nMaxAlphaValue = 100;    //Alpha值得最大值
int       g_nAlphaValueSlider;       //滑动条对应的变量
double    g_dAlphaValue;
double    g_dBetaValue;

//声明存储图像的变量
Mat g_srcImage1;
Mat g_srcImage2;
Mat g_dstImage;


void on_Trackbar(int position, void*)       //响应滑动条的回调函数
{
    g_dAlphaValue = (double)g_nAlphaValueSlider / g_nMaxAlphaValue;
    g_dBetaValue = 1.0 - g_dAlphaValue;
    
    //根据alpha和beta值进行线性混合
    addWeighted(g_srcImage1, g_dAlphaValue, g_srcImage2, g_dBetaValue, 0.0, g_dstImage);
    imshow(WINDOW_NAME, g_dstImage);
}

int main(int argc, char** agrv)
{
    g_srcImage1 = imread("1.jpg");
    g_srcImage2 = imread("2.jpg");

    g_nAlphaValueSlider = 70;            //设置滑动条的初值为70

    namedWindow(WINDOW_NAME, 1);         //创建窗口

    char TrackbarName[50];
    sprintf(TrackbarName, "透明值 %d", g_nMaxAlphaValue);

    createTrackbar(TrackbarName, WINDOW_NAME, &g_nAlphaValueSlider, g_nMaxAlphaValue, on_Trackbar);

    on_Trackbar(g_nAlphaValueSlider, 0);
    waitKey();
    return 0;
}