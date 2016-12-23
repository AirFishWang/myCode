#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    bool deleteH = true;                     //是否去除水平线
    bool deleteV = true;                     //是否去除垂直线
    Mat srcImage = imread(argv[1], IMREAD_GRAYSCALE);
    if (srcImage.data == NULL)
    {
        cout << "Can't find " << argv[1] << endl;
        return 0;
    }
    imshow("src", srcImage);
    int erosion_size = 35;
    Mat elementH = getStructuringElement(MORPH_RECT, Size(2 * erosion_size + 1, 1));      //水平掩模
    Mat elementV = getStructuringElement(MORPH_RECT, Size(1, 2 * erosion_size + 1));      //竖直掩模
    Mat lineImage;                           //线图
    if (deleteH == true && deleteV == false)
    {
        Mat dilateDstH;
        dilate(srcImage, dilateDstH, elementH);     //最大值卷积，白色膨胀
        //imwrite("dilateDstH.png", dilateDstH);
        //imshow("dilateDstH", dilateDstH);
        erode(dilateDstH, lineImage, elementH);     //最小值卷积，黑色膨胀(还原作用)
        //imwrite("lineImageH.png", lineImage);
        imshow("lineImageH", lineImage);            //水平线图
    }
    else if (deleteH == false && deleteV == true)
    {
        Mat dilateDstV;
        dilate(srcImage, dilateDstV, elementV);     //最大值卷积，白色膨胀
        //imwrite("dilateDstV.png", dilateDstH);
        //imshow("dilateDstV", dilateDstH);
        erode(dilateDstV, lineImage, elementV);     //最小值卷积，黑色膨胀(还原作用)
        //imwrite("lineImageV.png", lineImage);
        imshow("lineImageV", lineImage);            //垂直线图
    }
    else if (deleteH == true && deleteV == true)
    {
        Mat dilateDstH, erodeDstH;
        dilate(srcImage, dilateDstH, elementH);
        erode(dilateDstH, erodeDstH, elementH);     //erodeDstH水平线图

        Mat dilateDstV, erodeDstV;
        dilate(srcImage, dilateDstV, elementV);
        erode(dilateDstV, erodeDstV, elementV);     //erodeDstV垂直线图

        bitwise_and(erodeDstH, erodeDstV, lineImage);
        //imwrite("lineImageHV.png", lineImage);
        imshow("lineImageHV", lineImage);
    }
    else
    {
        cout << "The deleteH and deleteV both are false\n";
        return 0;
    }


    Mat deleteLine;                                 
    bitwise_xor(srcImage, lineImage, deleteLine);     //异或
    bitwise_not(deleteLine, deleteLine);              //取反

    imwrite("deleteLine.png", deleteLine);
    imshow("deleteLine", deleteLine);
    waitKey(0);
    return 0;
}