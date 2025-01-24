#version 460
uniform mat4 mvpMatrix;
in vec4 vertexPosition;

void main()
{
    // Calculate vertex position in screen space.
    gl_Position = mvpMatrix * vertexPosition;
}
