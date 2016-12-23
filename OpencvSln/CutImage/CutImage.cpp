#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;



int main()
{
    Mat src = imread("3.jpg", IMREAD_ANYCOLOR);
    if (src.data == NULL)
        cout << "there is no srcImage\n";
    //imshow("SRC", src);
    //Mat dst = src(Range(570, 640), Range(280, 1450));
    //imshow("ROI", dst);
    //waitKey();

    //imwrite("2.png", dst);
    return 0;
}
