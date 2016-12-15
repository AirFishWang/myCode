#include "cv.h"                             //  OpenCV 文件头
#include "highgui.h"
#include "cvaux.h"
#include "cxcore.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;
Mat rotateImage(Mat img, int degree)
{
    degree = -degree;
    double angle = degree  * CV_PI / 180.; // 弧度  
    double a = sin(angle), b = cos(angle);
    int width = img.cols;
    int height = img.rows;
    int width_rotate = int(height * fabs(a) + width * fabs(b));
    int height_rotate = int(width * fabs(a) + height * fabs(b));

    //旋转数组map
    // [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
    // [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]

    float map[6];
    Mat map_matrix = Mat(2, 3, CV_32F, map);
    // 旋转中心
    CvPoint2D32f center = cvPoint2D32f(width / 2, height / 2);
    CvMat map_matrix2 = map_matrix;
    cv2DRotationMatrix(center, degree, 1.0, &map_matrix2);
    map[2] += (width_rotate - width) / 2;
    map[5] += (height_rotate - height) / 2;
    Mat img_rotate;
    //对图像做仿射变换
    //CV_WARP_FILL_OUTLIERS - 填充所有输出图像的象素。
    //如果部分象素落在输入图像的边界外，那么它们的值设定为 fillval.
    //CV_WARP_INVERSE_MAP - 指定 map_matrix 是输出图像到输入图像的反变换，
    warpAffine(img, img_rotate, map_matrix, Size(width_rotate, height_rotate), 1, 0, Scalar(255,255,255));
    return img_rotate;
}

int main(int argc, char *argv[])
{
    int degree;
    Mat srcImg;
    srcImg = imread("ad.jpg");
    //namedWindow("原图像", 1);
    //imshow("原图像", m_SrcImg);
    //cout << "请输入旋转的度数：";
    //cin >> degree;
    //Mat m_ResImg = rotateImage(m_SrcImg, degree);
    //namedWindow("旋转后图像", 1);
    //imshow("旋转后图像", m_ResImg);
    //waitKey(0);

    for (int degree = 10; degree < 360; degree += 10)
    {
        Mat resultImg = rotateImage(srcImg, degree);
        stringstream ss;   //用于将int转string
        ss << degree;
        string outPath = ss.str() + ".jpg";
        imwrite(outPath, resultImg);
    }
}