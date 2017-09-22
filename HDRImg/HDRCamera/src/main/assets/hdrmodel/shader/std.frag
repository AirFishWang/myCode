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
  vec3 gridLoc = vec3(texCoord.x, texCoord.y, guide);
  vec4 row0 = texture(sAffineGridRow0, gridLoc);
  vec4 row1 = texture(sAffineGridRow1, gridLoc);
  vec4 row2 = texture(sAffineGridRow2, gridLoc);
  colorOut = clamp(vec3(dot(row0, rgba), dot(row1, rgba), dot(row2, rgba)), 0.0, 1.0);
}
