/**************************************************************************
霍夫变换方法存在的问题：
(1): 直线检测的阈值无法确定，不同的图片，不同的阈值会有不同的效果(可以用膨胀来解决)
     i:  当检测不到直线的时候，可以缩小阈值
     ii: 当检测到的直线过多的时候，可以增大阈值      
     必须考虑上述情况，因为如果检测出的直线过多的话，会影响后面的检测速度和正确性
     控制在2到100
(2): 存在干扰线问题，当干扰线较多的时候，将会使平均角度的计算误差过多
     (拟解决方案，先升序排序，然后求最大连续稳定区域，可以解决，但现在的复杂度是O(n*n))
     还是要改进为O(n)的时间复杂度,否则直线过多的时候会影响计算速度
(3): 旋转后图片旋转到可视区之外，导致图片被切割(已解决)
**************************************************************************/

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <direct.h> 
#include <io.h>

using namespace cv;
using namespace std;

void GetFiles(string path, vector<string>& files)
{
    //文件句柄  
    long   hFile = 0;
    //文件信息  
    struct _finddata_t fileinfo;
    string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            //如果是目录,迭代之  
            //如果不是,加入列表  
            if ((fileinfo.attrib &  _A_SUBDIR))
            {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    GetFiles(p.assign(path).append("\\").append(fileinfo.name), files);
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
                //cout << p.assign(path).append("\\").append(fileinfo.name) << endl;
            }
        } while (_findnext(hFile, &fileinfo) == 0);   //依据文件系统存放位置顺讯遍历下一个
        _findclose(hFile);
    }
}

Mat RotateImage(Mat img, int degree)              //顺时针绕中心旋转
{
    degree = -degree;
    double angle = degree  * CV_PI / 180.;        //弧度  
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
    warpAffine(img, img_rotate, map_matrix, Size(width_rotate, height_rotate), 1, 0, Scalar(255, 255, 255));
    return img_rotate;
}

bool SortByAngle(Vec2f line1, Vec2f line2)      //升序
{
    return line1[1] < line2[1];
}

//对非空升序序列求最大稳定区间  这里的复杂度是O(n*n)
int GetMaxStableRegion(const vector<Vec2f> lines, int &begin, int &end, double threshold)
{
    begin = 0;
    end = 0;
    for (int i = 0; i < lines.size(); i++)
        for (int j = i; j < lines.size(); j++)
        {
            if (lines[j][1] - lines[i][1] < threshold)
            {
                if (j - i > end - begin)
                {
                    begin = i;
                    end = j;
                }

            }
            else
                break;                       //因为是升序，所以后面的一定也会是大于阈值
        }
    return 0;
}

int RotateCorrect(string filename)
{
    Mat srcImage = imread(filename);
    //imshow("原始图", srcImage);
    //waitKey();
    Mat midImage, dstImage;
    Canny(srcImage, midImage, 50, 200, 3);
    Mat element = getStructuringElement(MORPH_RECT, Size(11, 11));
    dilate(midImage, midImage, element);       //白色膨胀
    Canny(midImage, midImage, 50, 200, 3);     //再次提取边缘
    cvtColor(midImage, dstImage, COLOR_GRAY2BGR);   //COLOR_GRAY2BGR 是为了在dstImage中使用彩色的线条来绘制直线
    vector<Vec2f> lines;
    int lineThreshold = 100;
    while (1)     //需要依据检测到的直线数量来调整阈值，反复进行检测
    {
        HoughLines(midImage, lines, 1, CV_PI / 180, lineThreshold);
        if (lines.size() == 0)
        {
            if (lineThreshold < 20)           //小于20的情况下都没有检测到直线，则认为没有直线
                break;
            else
                lineThreshold -= 20;          //尝试减小阈值，并再次检测
        }
        else if (lines.size() > 100)
            lineThreshold += 20;              //尝试增加阈值，并再次检测
        else
            break;                            //检测到的直线数量在0到100之间，认为是一个合理的数量，则结束检测
        //当直线的数量在0到100之间的时候，也会大大减少sort和getMaxStableRegion的时间
    };

    if (lines.size() == 0)
    {
        cout << "no line detected" << endl;
        return 0;
    }
    sort(lines.begin(), lines.end(), SortByAngle);      //升序
    int begin;
    int end;
    GetMaxStableRegion(lines, begin, end, 5 * CV_PI / 180);
    double averageAngle;
    double angleSum = 0.0;                     //极角和
    for (size_t i = begin; i <= end; i++)
    {
        float rho = lines[i][0];               //垂直距离   极径可能是负值
        float theta = lines[i][1];             //与x轴正方向的角度，0表示垂直线，pai/2 表示水平线,  极角的范围是[0, pai]
        angleSum += theta;                     //累积极角

        //绘制直线
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        int detal = 1000;
        Point pt1(cvRound(x0 - detal * b), cvRound(y0 + detal * a));
        Point pt2(cvRound(x0 + detal * b), cvRound(y0 - detal * a));

        line(dstImage, pt1, pt2, Scalar(0, 0, 255), 1, 8);
    }
    averageAngle = angleSum / (end - begin + 1);    //平均极角
    double angleThres = 5 * CV_PI / 180;            //偏差绝对值小于阈值将不做旋转
    //angleThres = 0.087266462599716474;

    if (fabs(averageAngle - CV_PI / 2) > angleThres)  //水平线的极径是CV_PI / 2  
    {
        Mat rotateImg = RotateImage(srcImage, 90 - averageAngle * 180 / CV_PI);
        int pos = filename.find_last_of("/\\");
        string outName = "./out/"+ filename.substr(pos + 1, filename.length() - pos - 1);    //文件名
        imwrite(outName, rotateImg);
    }

    return 0;
}

int main()
{
#if 0
    Mat srcImage = imread("1.jpg");
    //imshow("原始图", srcImage);
    Mat midImage, dstImage;
    Canny(srcImage, midImage, 50, 200, 3);
    //imshow("第一次边缘检测后的图", midImage);
    //waitKey();
    //imwrite("canny.jpg", midImage);            //midImage是单通道二值图(只有0和255)

    Mat element = getStructuringElement(MORPH_RECT, Size(11, 11)); 
    dilate(midImage, midImage, element);       //白色膨胀
    //imshow("膨胀图", midImage);
    //waitKey();
    //imwrite("dilate.jpg", midImage);
    Canny(midImage, midImage, 50, 200, 3);     //再次提取边缘
    //imshow("第二次边缘检测后的图", midImage);
    //waitKey();
    cvtColor(midImage, dstImage, COLOR_GRAY2BGR);   //COLOR_GRAY2BGR 是为了在dstImage中使用彩色的线条来绘制直线

    vector<Vec2f> lines;

    int lineThreshold = 80;
    while (1)     //需要依据检测到的直线数量来调整阈值，反复进行检测
    {
        HoughLines(midImage, lines, 1, CV_PI / 180, lineThreshold);
        if (lines.size() == 0)
        {
            if (lineThreshold < 20)           //小于20的情况下都没有检测到直线，则认为没有直线
                break;
            else
                lineThreshold -= 20;          //尝试减小阈值，并再次检测
        }
        else if (lines.size() > 100)
            lineThreshold += 20;              //尝试增加阈值，并再次检测
        else
            break;                            //检测到的直线数量在0到100之间，认为是一个合理的数量，则结束检测
                                              //当直线的数量在0到100之间的时候，也会大大减少sort和getMaxStableRegion的时间
    };

    if (lines.size() == 0)
    {
        cout << "no line detected" << endl;
        return 0;
    }

    auto nStart = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    sort(lines.begin(), lines.end(), SortByAngle);      //升序
    auto nEnd = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    printf("sort Time: %d ms\n", nEnd - nStart);

    int begin;
    int end;

    nStart = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    getMaxStableRegion(lines, begin, end, 5 * CV_PI / 180);
    nEnd = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    printf("getMaxStableRegion Time: %d ms\n", nEnd - nStart);

    double averageAngle;
    double angleSum = 0.0;                     //极角和
    for (size_t i = begin; i <= end; i++)
    {
        float rho = lines[i][0];               //垂直距离   极径可能是负值
        float theta = lines[i][1];             //与x轴正方向的角度，0表示垂直线，pai/2 表示水平线,  极角的范围是[0, pai]
        angleSum += theta;                     //累积极角

        //绘制直线
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        int detal = 1000;
        Point pt1(cvRound(x0 - detal * b), cvRound(y0 + detal * a));
        Point pt2(cvRound(x0 + detal * b), cvRound(y0 - detal * a));

        line(dstImage, pt1, pt2, Scalar(0, 0, 255), 1, 8);
    }
    imshow("霍夫检测效果图", dstImage);
   // imwrite("hough.jpg", dstImage);
    waitKey();

    averageAngle = angleSum / (end - begin + 1);    //平均极角
    double angleThres = 5 * CV_PI / 180;            //偏差绝对值小于阈值将不做旋转
                                                    //angleThres = 0.087266462599716474;

    if (fabs(averageAngle - CV_PI / 2) > angleThres)  //水平线的极径是CV_PI / 2  
    {
        Mat rotateImg = rotateImage(srcImage, 90 - averageAngle * 180  / CV_PI);
        imshow("旋转图", rotateImg);
        waitKey();
        imwrite("rotateImage.jpg", rotateImg);
    }
#else              //批处理
    _mkdir("out");
    vector<string> files;
    GetFiles("ad", files);             //获取该路径下的所有文件  
    int count = files.size();           //总的图片数量
    for (int i = 0; i < count; i++)
    {
        cout << files[i].c_str() << endl;
        RotateCorrect(files[i]);
    }



#endif

    return 0;
}