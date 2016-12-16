#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

#define WINDOW_NAME "【通道分割】"

//srcImage永远保持不变
typedef struct RGBThreshold
{
    const Mat srcImage;
    int rThreshold;
    int gThreshold;
    int bThreshold;
}RGBThreshold;

Mat RgbFilter(RGBThreshold *data);

void onRTrackbar(int position, void* data)       //响应滑动条的回调函数
{
    RGBThreshold *p = (RGBThreshold *)data;
    RgbFilter(p);
}

void onGTrackbar(int position, void* data)       //响应滑动条的回调函数
{
    RGBThreshold *p = (RGBThreshold *)data;
    RgbFilter(p);
}

void onBTrackbar(int position, void* data)       //响应滑动条的回调函数
{
    RGBThreshold *p = (RGBThreshold *)data;
    RgbFilter(p);
}

Mat RgbFilter(RGBThreshold *data)    //注意Mat的存储顺序是 B G R
{
    Mat dstImage = data->srcImage.clone();

    for (int i = 0; i < dstImage.rows; i++)
        for (int j = 0; j < dstImage.cols; j++)
        {
            if (data->srcImage.at<Vec3b>(i, j)[2] <= data->rThreshold && 
                data->srcImage.at<Vec3b>(i, j)[1] <= data->gThreshold &&
                data->srcImage.at<Vec3b>(i, j)[0] <= data->bThreshold
                )
            {
                dstImage.at<Vec3b>(i, j)[2] = 0;
                dstImage.at<Vec3b>(i, j)[1] = 0;
                dstImage.at<Vec3b>(i, j)[0] = 0;
            }  
            else
            {
                dstImage.at<Vec3b>(i, j)[2] = 255;
                dstImage.at<Vec3b>(i, j)[1] = 255;
                dstImage.at<Vec3b>(i, j)[0] = 255;
            }
        }
    imshow(WINDOW_NAME, dstImage);
    return dstImage;
}

int main(int argc, char** agrv)
{
    Mat srcImage = imread("fuwu.jpg");
    //Mat srcImage = imread("2.jpg");
    RGBThreshold thres{ srcImage, 200, 200, 180 };
    namedWindow(WINDOW_NAME, 1);         //创建窗口


    createTrackbar("R ", WINDOW_NAME, &thres.rThreshold, 255, onRTrackbar, &thres);
    createTrackbar("G ", WINDOW_NAME, &thres.gThreshold, 255, onGTrackbar, &thres);
    createTrackbar("B ", WINDOW_NAME, &thres.bThreshold, 255, onBTrackbar, &thres);


    onRTrackbar(thres.rThreshold, &thres);
    onGTrackbar(thres.gThreshold, &thres);
    onBTrackbar(thres.bThreshold, &thres);
    waitKey();
    destroyAllWindows();

    Mat result = RgbFilter(&thres);
    imwrite("result.jpg", result);
    return 0;
}