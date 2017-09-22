#include "renderer.h"

#include <cstdio>
#include <chrono>
#include <thread>

#include <android/log.h>
#define LOG_TAG "renderer.cpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "utils.h"

void Renderer::initpTexCoord(int cameraIndex)
{
    //float pVertex_[8] = { 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
    if(cameraIndex == 0) //0: 后置摄像头
    {
        pTexCoord_[0] = 1.0f; pTexCoord_[1] = 0.0f;
        pTexCoord_[2] = 1.0f; pTexCoord_[3] = 1.0f;
        pTexCoord_[4] = 0.0f; pTexCoord_[5] = 0.0f;
        pTexCoord_[6] = 0.0f; pTexCoord_[7] = 1.0f;
    }
    else                 //1: 前置摄像头
    {
        pTexCoord_[0] = 0.0f; pTexCoord_[1] = 0.0f;
        pTexCoord_[2] = 0.0f; pTexCoord_[3] = 1.0f;
        pTexCoord_[4] = 1.0f; pTexCoord_[5] = 0.0f;
        pTexCoord_[6] = 1.0f; pTexCoord_[7] = 1.0f;
    }
    
    __android_log_print(ANDROID_LOG_INFO, "initpTexCoord", "%d", cameraIndex);
}

Renderer::Renderer(int output_width, int output_height,
    int grid_width, int grid_height, int grid_depth,
    std::string vertex_shader_path, std::string fragment_shader_path,
    std::string checkpoint_path, int cameraIndex)
  : output_width_(output_width), output_height_(output_height),
    grid_width_(grid_width), grid_height_(grid_height), grid_depth_(grid_depth)
{  
    
    initpTexCoord(cameraIndex);
    char vShaderStr[] = 
            "#version 300 es                                            \n"
            "in vec2 vPosition;                                         \n"                                                       
            "in vec2 vTexCoord;                                         \n"
            "out vec2 texCoord;                                         \n"
            "void main() {                                              \n"       
            "    vec4 homogeneous = vec4(vTexCoord.xy, 0, 1);           \n"
            "    texCoord = vTexCoord;                                  \n"
            "    gl_Position = vec4(vPosition.x, vPosition.y, 0.0, 1.0);\n"
            "}                                                          \n";
            
    char fShaderStr[] =
            "#version 300 es                                            \n"
            "precision lowp float;                                      \n"
            "layout(location = 1) uniform lowp sampler2D sRGB;               \n"
            "layout(location = 2) uniform lowp sampler3D sAffineGridRow0;    \n"
            "layout(location = 3) uniform lowp sampler3D sAffineGridRow1;    \n"
            "layout(location = 4) uniform lowp sampler3D sAffineGridRow2;    \n"
            "uniform mat3x4 uGuideCcm;                                  \n"
            "uniform vec3 uGuideShifts[16];                             \n"
            "uniform vec3 uGuideSlopes[16];                             \n"
            "uniform vec4 uMixMatrix;                                   \n"
            "in vec2 texCoord;                                          \n"
            "layout(location = 0) out vec3 colorOut;                    \n"
            "void main() {                                              \n"
            "  vec2 offset_x = vec2(1.0, 0.0)/2048.0;                   \n"
            "  vec2 offset_y = vec2(0.0, 1.0)/2048.0;                   \n"
            "  vec4 rgba = vec4(texture(sRGB, texCoord).xyz, 1.0);      \n"
            "  vec3 tmp = (rgba*uGuideCcm);                             \n"
            "  vec3 tmp2 = vec3(0);                                     \n"
            "  for (int i = 0; i < 16; ++i) {                           \n"
            "    tmp2 += uGuideSlopes[i].rgb * max(vec3(0), tmp - uGuideShifts[i].rgb);\n"
            "  }                                                            \n"
            "  float guide = clamp(dot(vec4(tmp2, 1.0), uMixMatrix), 0.0, 1.0); \n"
            "  vec3 gridLoc = vec3(texCoord.x, texCoord.y, guide);          \n"
            "  vec4 row0 = texture(sAffineGridRow0, gridLoc);               \n"
            "  vec4 row1 = texture(sAffineGridRow1, gridLoc);               \n"
            "  vec4 row2 = texture(sAffineGridRow2, gridLoc);               \n"
            "  colorOut = clamp(vec3(dot(row0, rgba), dot(row1, rgba), dot(row2, rgba)), 0.0, 1.0);\n"
            "}                                                              \n";
      
    vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
  
  LOGI("%s", vertex_shader_path.c_str());
  shader_from_file(vertex_shader_path, vertex_shader_);
  shader_from_file(fragment_shader_path, fragment_shader_);
  //shader_from_char(vShaderStr, vertex_shader_);
  //shader_from_char(fShaderStr, fragment_shader_);
  
  // Create program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader_);
  glAttachShader(program_, fragment_shader_);
  glLinkProgram(program_);
  GLint link_success;
  glGetProgramiv(program_, GL_LINK_STATUS, &link_success);
  if (link_success == GL_FALSE) {
    __android_log_print(ANDROID_LOG_INFO, "renderer.cpp", "Failed to link program"); 
    GLint logSize = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logSize);
    std::vector<GLchar> errorLog(logSize);
    glGetProgramInfoLog(program_, logSize, &logSize, &errorLog[0]);
    std::cout << errorLog.data() << std::endl;
    glDeleteProgram(program_);
    program_ = 0;
    throw;
  }
  else 
    __android_log_print(ANDROID_LOG_INFO, "renderer.cpp", "link program successed"); 
  glUseProgram(program_);

  // Geometry
  int ph = glGetAttribLocation(program_, "vPosition");
  int tch = glGetAttribLocation(program_, "vTexCoord");
  glVertexAttribPointer(ph, 2, GL_FLOAT, false, 4*2, static_cast<GLvoid*>(pVertex_));
  glVertexAttribPointer(tch, 2, GL_FLOAT, false, 4*2, static_cast<GLvoid*>(pTexCoord_));
  glEnableVertexAttribArray(ph);
  glEnableVertexAttribArray(tch);

  // Output texture
  glGenTextures(1, &output_texture_);
  glBindTexture(GL_TEXTURE_2D, output_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, output_width_, output_height_, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);  //设置可变存储
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Input texture
  glGenTextures(1, &input_texture_);
  glBindTexture(GL_TEXTURE_2D, input_texture_);   //纹理的维度 GL_TEXTURE_2D
  glTexStorage2D(GL_TEXTURE_2D, 1 , GL_RGB8, output_width_, output_height_);   //创建纹理存储
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);         //纹理过滤函数glTexParameteri
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   
  glUniform1i(glGetUniformLocation(program_, "sRGB"), 1);   //将1号纹理单元作为整数传递给片元着色器，片元着色器中使用uniform变量sRGB对应这个纹理采样器
  //glGetUniformLocation: 返回着色器中uniform变量sRGB的对应索引值
  
/*
  // Output framebuffer
  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);    //可读可写,分配帧缓存对象的存储空间并初始化 
  //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, output_texture_, 0);  //FBO本身没有图像存储区。我们必须帧缓存关联图像（纹理或渲染对象）关联到FBO。这种机制允许FBO快速地切换（分离和关联）帧缓存关联图像
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_texture_, 0);
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};   //GL_COLOR_ATTACHMENT0 第0个颜色缓存
  glDrawBuffers(1, draw_buffers); // "1" is the size of DrawBuffers   选择要写入的缓存 p147(红皮书)

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Frame buffer did not complete operation" << std::endl;
    throw;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  */
  //glViewport(0, 0, output_width_, output_height_);
}

void Renderer::render(const float* const coeffs_data, cv::Mat & output,
        double *upload_coeff_time, double *draw_time, double *readback_time) {


  // Upload coefficient grid to GPU

  upload_coefficients(coeffs_data); 
  glClearColor ( 1.0f, 1.0f, 1.0f, 0.0f );
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 


  //glReadPixels(0, 0, output_width_, output_height_,
  //             GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) output.data);  //像素数据的读取 存入到output.date中
        
}


void Renderer::upload_input(const cv::Mat &image) {
  // Upload input image.
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, input_texture_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, output_width_, output_height_,
                  GL_RGB, GL_UNSIGNED_BYTE, image.data);
}

StandardRenderer::StandardRenderer(
    int output_width, int output_height,
    int grid_width, int grid_height, int grid_depth,
    std::string vertex_shader, std::string fragment_shader,
    std::string checkpoint_path, int cameraIndex) :
  Renderer(output_width, output_height,
      grid_width, grid_height, grid_depth,
      vertex_shader, fragment_shader,
      checkpoint_path,cameraIndex)
{
  
  // Bind affine coefficients to three texture samplers, one row each.
  gl_extra_setup();    //创建3D纹理，当tensorflow计算出系数矩阵时，将数据绑定到纹理上
  load_guide_parameters(checkpoint_path);
  
  LOGI("gl_extra_setup and load_guide_parameters finish");
};

void StandardRenderer::load_guide_parameters(std::string checkpoint_path) {
  float ccm[3*4] = {0};
  float mix_matrix[4*1] = {0};
  float shifts[16*3] = {0};
  float slopes[16*3] = {0};

  load_binary_data(checkpoint_path+"guide_ccm_f32_3x4.bin", 3*4, ccm);
  load_binary_data(checkpoint_path+"guide_mix_matrix_f32_1x4.bin", 4, mix_matrix);
  load_binary_data(checkpoint_path+"guide_shifts_f32_16x3.bin", 16*3, shifts);
  load_binary_data(checkpoint_path+"guide_slopes_f32_16x3.bin", 16*3, slopes);
    glUniformMatrix3x4fv(glGetUniformLocation(program_, "uGuideCcm"),1, false, ccm);
    glUniform4fv(glGetUniformLocation(program_, "uMixMatrix"),1, mix_matrix);
    glUniform3fv(glGetUniformLocation(program_, "uGuideShifts"),16, shifts);
    glUniform3fv(glGetUniformLocation(program_, "uGuideSlopes"),16, slopes);
}


void StandardRenderer::gl_extra_setup() {
  glGenTextures(3, coeffs_textures_);
  for (int i = 0; i < 3; ++i) {
    glBindTexture(GL_TEXTURE_3D, coeffs_textures_[i]);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, grid_width_, grid_height_, grid_depth_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glUniform1i(glGetUniformLocation(program_, "sAffineGridRow0"), 2);
  glUniform1i(glGetUniformLocation(program_, "sAffineGridRow1"), 3);
  glUniform1i(glGetUniformLocation(program_, "sAffineGridRow2"), 4);
}

void StandardRenderer::upload_coefficients(const float* const coeffs_data) {
  for (int i = 0; i < 3; ++i) {
    //LOGI("upload_coefficients for i = %d", i);
    glActiveTexture(GL_TEXTURE2 + i);  //激活纹理单元, 绑定纹理对象 GL_TEXTURE2 = GL_TEXTURE0 + 2
    glBindTexture(GL_TEXTURE_3D, coeffs_textures_[i]);
    glTexSubImage3D(GL_TEXTURE_3D,     //更新纹理数据
            0, // level
            0, 0, 0, // x, y, z offset
            grid_width_, grid_height_, grid_depth_,
            GL_RGBA, GL_FLOAT, coeffs_data + i*grid_width_*grid_height_*grid_depth_*4);
  }
  //LOGI("upload_coefficients end");
}

Renderer::~Renderer() {
  // GL cleanup
  //glDeleteQueries(kNumQueries, query_ids_);
  glDetachShader(program_, vertex_shader_);
  glDetachShader(program_, fragment_shader_);
  glDeleteShader(vertex_shader_);
  glDeleteShader(fragment_shader_);
  glDeleteProgram(program_);
  glDeleteTextures(1, &output_texture_);
  glDeleteTextures(1, &input_texture_);
  glDeleteFramebuffers(1, &framebuffer_);
}
