#version 460
uniform mat4 mvpMatrix;
uniform float currentStep;
in vec4 vertexPosition;
smooth out vec2 texCoord;

void main()
{
    // Calculate vertex position in screen space.
    gl_Position = mvpMatrix * vec4(vertexPosition.x, vertexPosition.y, currentStep, vertexPosition.w);
    texCoord = vec2(vertexPosition.x, vertexPosition.y);
}
