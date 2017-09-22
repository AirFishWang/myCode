#include <algorithm>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <gflags/gflags.h>

#include "timer.h"
#include "processor.h"
#include "utils.h"
using namespace std;

bool use_gpu = false;
int burn_iters = 2;         //"Iterations to run without benchmarking."
int iters = 2;              //"Benchmark averaging iterations."
string checkpoint_path = "/root/Desktop/hdrnet/pretrained_models/hdrp";
string mode = "HDRNetCurves";

int main(int argc, char *argv[])
{
  std::string root = argv[0];
  root = root.substr(0, root.find_last_of("/"));
  root = root.substr(0, root.find_last_of("/"))+"/";
  if (FLAGS_checkpoint_path.empty()) {
      std::cerr << "--checkpoint_path is required." << std::endl;
      return 1;
  }
  std::string model_name = checkpoint_path.substr(checkpoint_path.find_last_of("/")+1, checkpoint_path.size()-1);
  checkpoint_path += "/";

  cv::Mat image = load_image(input_path);

  int image_width = image.size().width;
  int image_height = image.size().height;

  Processor *processor = nullptr;
  processor = new StandardProcessor(image_width, image_height, checkpoint_path, use_gpu, root+"assets/");

  cv::Mat output_rgb(image_height, image_width, CV_8UC3, cv::Scalar(0));
  // Discard first few iterations.
  for (int i = 0; i < burn_iters; ++i) {
    processor->process(image, output_rgb);
  }
  // -- Processing ------------------------------
  BenchmarkResult result;
  for (int i = 0; i < iters; ++i) {
    //printf("Running actual benchmark: iteration %d of %d.\r", i, iters);
    result = result + processor->process(image, output_rgb);
  }

  cv::Mat output_bgr;
  cv::cvtColor(output_rgb, output_bgr, CV_RGB2BGR, 3);

  delete processor;
  return 0;
}
