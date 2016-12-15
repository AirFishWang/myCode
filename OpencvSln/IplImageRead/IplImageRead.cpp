#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main()
{
    string filename = "C:\\Users\\wangchun\\Desktop\\t2.png";
    IplImage *srcImage = cvLoadImage(filename.c_str(), 1);   
    //cvShowImage("src", srcImage);
    //waitKey();
    CvSize s = cvGetSize(srcImage);
    cout << "s.width = " << s.width << endl;
    cout << "s.height = " << s.height << endl;
}