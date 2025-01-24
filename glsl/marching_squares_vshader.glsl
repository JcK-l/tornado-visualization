#version 460
layout (location = 0) in vec3 vertexPosition;


layout (location = 1) in vec3 vInData;

out VS_OUT
{
  vec3 vData;
} vs_out;


void main()
{
  // Calculate vertex position in screen space.
  gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1.0);
  vs_out.vData = vInData;
}
