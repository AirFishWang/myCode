#version 300 es
precision lowp float;
layout(location = 1) uniform lowp sampler2D sRGB;
layout(location = 2)uniform lowp sampler3D sAffineGridRow0;
layout(location = 3) uniform lowp sampler3D sAffineGridRow1;
layout(location = 4) uniform lowp sampler3D sAffineGridRow2;
uniform mat3x4 uGuideCcm;
uniform vec3 uGuideShifts[16];
uniform vec3 uGuideSlopes[16];
uniform vec4 uMixMatrix;
in vec2 texCoord;
layout(location = 0) out vec3 colorOut;

void main() {
  vec2 offset_x = vec2(1.0, 0.0)/2048.0;
  vec2 offset_y = vec2(0.0, 1.0)/2048.0;
  vec4 rgba = vec4(texture(sRGB, texCoord).xyz, 1.0);
  vec3 tmp = (rgba*uGuideCcm);
  vec3 tmp2 = vec3(0);
  for (int i = 0; i < 16; ++i) {
    tmp2 += uGuideSlopes[i].rgb * max(vec3(0), tmp - uGuideShifts[i].rgb);
  }
  float guide = clamp(dot(vec4(tmp2, 1.0), uMixMatrix), 0.0, 1.0);
  vec3 gridLoc = vec3(texCoord.x, texCoord.y, guide);    //三线性插值的作用
  vec4 row0 = texture(sAffineGridRow0, gridLoc);
  vec4 row1 = texture(sAffineGridRow1, gridLoc);
  vec4 row2 = texture(sAffineGridRow2, gridLoc);
  colorOut = clamp(vec3(dot(row0, rgba), dot(row1, rgba), dot(row2, rgba)), 0.0, 1.0);
}

/*
sRGB:  原图像I
sAffineGridRow[012]: 16 *16 * 8的3D纹理， 且纹素为rgba四个分量，由tensorflow产生
uGuideCcm:  用于产生图像g的矩阵M
uGuideShifts: Tc,i 
uGuideSlopes: Ac,i
uMixMatrix  : 用于求和来产生灰度值g(x,y)，也就是guide
row0 = [A0, A1, A2, A3]
row1 = [A4, A5, A6, A7]
row2 = [A8, A9, A10, A11]

O0 = A3 + A0*I0 + A1*I1 + A2*I2
O1 = A7 + A4*I0 + A5*I1 + A6*I2
O2 = A11 + A8*I0 + A9*I1 + A10*I2

*/
