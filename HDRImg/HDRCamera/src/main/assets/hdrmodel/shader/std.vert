#version 300 es

in vec2 vPosition;
in vec2 vTexCoord;
out vec2 texCoord;

void main() {
  vec4 homogeneous = vec4(vTexCoord.xy, 0, 1);
  texCoord = vTexCoord;
  gl_Position = vec4(vPosition.x, vPosition.y, 0.0, 1.0);
}
