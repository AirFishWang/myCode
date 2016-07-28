#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdint.h>
#include <direct.h>
#include <io.h>

using namespace cv;
using namespace std;

void getFiles(string path, vector<string>& files)
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
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);   //依据文件系统存放位置顺讯遍历下一个
		_findclose(hFile);
	}
}

void main()
{
/*	Mat img = imread("1.bmp");
	resize(img, img, Size(24, 24));   //归一化为24×24的图像
	cvtColor(img, img, CV_RGB2GRAY);  //转换为灰度图
	equalizeHist(img, img);           //直方图均衡处理，增强对比度
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 10; j++)
			printf("%d\t", (uint8_t)(img.at<uchar>(i, j)));
			//cout << (uint8_t)(img.at<uchar>(i, j)) << '\t';
		cout << endl;
	}
	
	uint8_t x = 255;
	cout << x << endl;
	printf("%d\n", x);

	imwrite("24.jpg", img);
	waitKey();
*/
	string foldername = "ZEX";
	vector<string> files;
	getFiles(foldername, files);		////获取该路径下的所有文件  
	int count = files.size();           //总的图片数量

	string dstFoldername = foldername + "bmp";
	_mkdir(dstFoldername.c_str());   //新建目标文件夹,用于存储预处理后bmp图像

	for (int i = 0; i < count; i++)
	{
		string path = files[i].c_str();
		Mat srcImage = imread(path), dstImage;
		resize(srcImage, dstImage, Size(32, 32));   //归一化为80×80的图像
		cvtColor(dstImage, dstImage, CV_RGB2GRAY);  //转换为灰度图
		equalizeHist(dstImage, dstImage);      //直方图均衡处理，增强对比度

		stringstream ss;   //用于将int转string
		ss << i + 1;
		path = dstFoldername + "\\" + ss.str() + ".bmp";   //写入路径
	
		imwrite(path, dstImage);                           //将处理后的图片写入到dstFoldername文件夹下

	}
}