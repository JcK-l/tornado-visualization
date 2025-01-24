#version 460
in vec4 vertexPosition;

void main()
{
    // Calculate vertex position in screen space.
    gl_Position = vertexPosition;
}
