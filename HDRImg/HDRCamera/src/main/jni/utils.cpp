// Copyright 2016 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "utils.h"
#include <android/log.h>

//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

cv::Mat load_image(std::string input_path) {
  cv::Mat image = cv::imread(input_path, CV_LOAD_IMAGE_COLOR);
  cv::Mat image_rgb;
  cv::cvtColor(image, image_rgb, CV_BGR2RGB, 3);
  return image_rgb;
}

void load_binary_data(std::string filename, int length, float* output) {
  std::ifstream file;
  file.open(filename, std::ios::in);
  if(!file) {
    //std::cout << "Failed to load file " << filename << std::endl;
    throw;
  }
  file.read((char*) output, sizeof(float)*length);
  file.close();
}

std::string readFileToString(const std::string &source)
{
    std::ifstream stream(source.c_str());
    if (!stream)
    {
        __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "Failed to load shader file %s", source.c_str()); 
        return "";
    }

    std::string result;
    stream.seekg(0, std::ios::end);
    result.reserve(static_cast<unsigned int>(stream.tellg()));
    stream.seekg(0, std::ios::beg);
    result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return result;
}

void shader_from_char(const char *shaderArray, GLuint& shader){

  glShaderSource(shader, 1, &shaderArray, NULL);
  glCompileShader(shader);
  

  GLint shader_success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_success);
  if (shader_success == GL_FALSE) {
    __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "Failed to compile shader"); 
    __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "%d", glGetError());
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if(logSize > 1){
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);
        __android_log_print(ANDROID_LOG_INFO, "utils.h", "%s", &errorLog[0]); 
    }
    else 
        __android_log_print(ANDROID_LOG_INFO, "utils.h", "the length of log is %d", logSize); 

    glDeleteShader(shader);
    throw;
  }
}

void shader_from_file(const std::string filename, GLuint& shader) {
    
  std::ifstream file;
  file.open(filename, std::ios::in);
  if(!file) {
     __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "Failed to load shader file"); 
    throw;
  }
  
  std::string content;
  file.seekg(0, std::ios::end);
  content.reserve(file.tellg());
  file.seekg(0, std::ios::beg);
  content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  const GLchar* source = (const GLchar*) content.c_str();
  //__android_log_print(ANDROID_LOG_INFO, "utils.cpp", "%s", content.c_str()); 
  //__android_log_print(ANDROID_LOG_INFO, "utils.cpp", "%d", content.length()); 
 
  //__android_log_print(ANDROID_LOG_INFO, "utils.cpp", "begin glCompileShader");         
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  

  GLint shader_success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_success);
  if (shader_success == GL_FALSE) {
    __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "Failed to compile shader"); 
    __android_log_print(ANDROID_LOG_INFO, "utils.cpp", "%d", glGetError());
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if(logSize > 1){
        std::vector<GLchar> errorLog(logSize);
        glGetShaderInfoLog(shader, logSize, &logSize, &errorLog[0]);
    }
    else 
        __android_log_print(ANDROID_LOG_INFO, "utils.h", "the length of log is %d", logSize); 

    glDeleteShader(shader);
    throw;
  }
}
